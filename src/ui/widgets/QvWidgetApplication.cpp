#include "QvWidgetApplication.hpp"

#include "base/QvBase.hpp"
#include "components/translations/QvTranslator.hpp"
#include "core/settings/SettingsBackend.hpp"
#include "ui/widgets/styles/StyleManager.hpp"
#include "ui/widgets/windows/w_MainWindow.hpp"
#include "utils/QvHelpers.hpp"

#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QUrlQuery>

#ifdef Q_OS_WIN
#include <Winbase.h>
#endif

#define QV_MODULE_NAME "WidgetApplication"

constexpr auto QV2RAY_WIDGETUI_STATE_FILENAME = "UIState.json";

Qv2rayWidgetApplication::Qv2rayWidgetApplication(int &argc, char *argv[]) : QvPlatformApplication(argc, argv)
{
}

QStringList Qv2rayWidgetApplication::checkPrerequisitesInternal()
{
    return {};
}

void Qv2rayWidgetApplication::terminateUIInternal()
{
    delete mainWindow;
    delete hTray;
    delete StyleManager;
    StringToFile(JsonToString(UIStates), QVMESSOCKET_CONFIG_DIR + QV2RAY_WIDGETUI_STATE_FILENAME);
}

#ifndef QVMESSOCKET_NO_SINGLEAPPLICATON
void Qv2rayWidgetApplication::onMessageReceived(quint32 clientId, QByteArray _msg)
{
    // Sometimes SingleApplication will send message with clientId == 0, ignore them.
    if (clientId == instanceId())
        return;

    if (!isInitialized)
        return;

    const auto msg = QvStartupArguments::fromJson(JsonFromString(_msg));
    LOG("Client ID:", clientId, ", message received, version:", msg.buildVersion);
    DEBUG(_msg);
    //
    if (msg.buildVersion > QCLASH_VERSION_BUILD)
    {
        const auto newPath = msg.fullArgs.first();
        QString message;
        message += tr("A new version of Qclash is starting:") + NEWLINE;
        message += NEWLINE;
        message += tr("New version information: ") + NEWLINE;
        message += tr("Version: %1:%2").arg(msg.version).arg(msg.buildVersion) + NEWLINE;
        message += tr("Path: %1").arg(newPath) + NEWLINE;
        message += NEWLINE;
        message += tr("Do you want to exit and launch that new version?");

        const auto result = QvMessageBoxAsk(nullptr, tr("New version detected"), message);
        if (result == Yes)
        {
            StartupArguments._qvNewVersionPath = newPath;
            SetExitReason(EXIT_NEW_VERSION_TRIGGER);
            QCoreApplication::quit();
        }
    }

    for (const auto &argument : msg.arguments)
    {
        switch (argument)
        {
            case QvStartupArguments::EXIT:
            {
                SetExitReason(EXIT_NORMAL);
                quit();
                break;
            }
            case QvStartupArguments::NORMAL:
            {
                mainWindow->show();
                mainWindow->raise();
                mainWindow->activateWindow();
                break;
            }
            case QvStartupArguments::RECONNECT:
            {
                ConnectionManager->RestartConnection();
                break;
            }
            case QvStartupArguments::DISCONNECT:
            {
                ConnectionManager->StopConnection();
                break;
            }
            case QvStartupArguments::QV_LINK:
            {
                for (const auto &link : msg.links)
                {
                    const auto url = QUrl::fromUserInput(link);
                    const auto command = url.host();
                    auto subcommands = url.path().split("/");
                    subcommands.removeAll("");
                    QMap<QString, QString> args;
                    for (const auto &kvp : QUrlQuery(url).queryItems())
                    {
                        args.insert(kvp.first, kvp.second);
                    }
                    if (command == "open")
                    {
                        emit mainWindow->ProcessCommand(command, subcommands, args);
                    }
                }
                break;
            }
        }
    }
}
#endif

QvExitReason Qv2rayWidgetApplication::runQvmessocketInternal()
{
    setQuitOnLastWindowClosed(false);
    hTray = new QSystemTrayIcon();
    StyleManager = new QvStyleManager();
    StyleManager->ApplyStyle(GlobalConfig.uiConfig.theme);
    // Show MainWindow
    UIStates = JsonFromString(StringFromFile(QVMESSOCKET_CONFIG_DIR + QV2RAY_WIDGETUI_STATE_FILENAME));
    mainWindow = new MainWindow();
    if (StartupArguments.arguments.contains(QvStartupArguments::QV_LINK))
    {
        for (const auto &link : StartupArguments.links)
        {
            const auto url = QUrl::fromUserInput(link);
            const auto command = url.host();
            auto subcommands = url.path().split("/");
            subcommands.removeAll("");
            QMap<QString, QString> args;
            for (const auto &kvp : QUrlQuery(url).queryItems())
            {
                args.insert(kvp.first, kvp.second);
            }
            if (command == "open")
            {
                emit mainWindow->ProcessCommand(command, subcommands, args);
            }
        }
    }
    isInitialized = true;
    return (QvExitReason) exec();
}

void Qv2rayWidgetApplication::OpenURL(const QString &url)
{
    QDesktopServices::openUrl(url);
}

void Qv2rayWidgetApplication::MessageBoxWarn(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox::warning(parent, title, text, QMessageBox::Ok);
}

void Qv2rayWidgetApplication::MessageBoxInfo(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox::information(parent, title, text, QMessageBox::Ok);
}

MessageOpt Qv2rayWidgetApplication::MessageBoxAsk(QWidget *parent, const QString &title, const QString &text, const QList<MessageOpt> &buttons)
{
    QFlags<QMessageBox::StandardButton> btns;
    for (const auto &b : buttons)
    {
        btns.setFlag(MessageBoxButtonMap[b]);
    }
    return MessageBoxButtonMap.key(QMessageBox::question(parent, title, text, btns));
}

void Qv2rayWidgetApplication::ShowTrayMessage(const QString &m, int msecs)
{
    hTray->showMessage("Qv2ray", m, QIcon(":/assets/icons/qv2ray.png"), msecs);
}
