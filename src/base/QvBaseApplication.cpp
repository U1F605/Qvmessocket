#include "QvBaseApplication.hpp"

#include "components/translations/QvTranslator.hpp"
#include "core/settings/SettingsBackend.hpp"
#include "utils/QvHelpers.hpp"

#define QV_MODULE_NAME "BaseApplication"
inline QString makeAbs(const QString &p)
{
    return QDir(p).absolutePath();
}

QvApplicationInterface::QvApplicationInterface()
{
    ConfigObject = new QvConfigObject;
    QvCoreApplication = this;
    LOG(QCLASH_VERSION_STRING, "on", QSysInfo::prettyProductName(), QSysInfo::currentCpuArchitecture());
    DEBUG("Start Time: ", QTime::currentTime().msecsSinceStartOfDay());
    DEBUG("QVMESSOCKET_BUILD_INFO", QVMESSOCKET_BUILD_INFO);
    DEBUG("QVMESSOCKET_BUILD_EXTRA_INFO", QVMESSOCKET_BUILD_EXTRA_INFO);
    DEBUG("QVMESSOCKET_BUILD_NUMBER", QSTRN(QCLASH_VERSION_BUILD));
    QStringList licenseList;
    licenseList << "This program comes with ABSOLUTELY NO WARRANTY.";
    licenseList << "This is free software, and you are welcome to redistribute it";
    licenseList << "under certain conditions.";
    licenseList << "Copyright (c) 2019-2021 Qv2ray Development Group.";
    licenseList << "Third-party libraries that have been used in this program can be found in the About page.";
    LOG(licenseList.join(NEWLINE));
}

QvApplicationInterface::~QvApplicationInterface()
{
    delete ConfigObject;
    QvCoreApplication = nullptr;
}

QStringList QvApplicationInterface::GetAssetsPaths(const QString &dirName) const
{
    // Configuration Path
    QStringList list;

    if (qEnvironmentVariableIsSet("QV2RAY_RESOURCES_PATH"))
        list << makeAbs(qEnvironmentVariable("QV2RAY_RESOURCES_PATH") + "/" + dirName);

    // Default behavior on Windows
    list << makeAbs(QCoreApplication::applicationDirPath() + "/" + dirName);
    list << makeAbs(QVMESSOCKET_CONFIG_DIR + dirName);
    list << ":/" + dirName;

    list << QStandardPaths::locateAll(QStandardPaths::AppDataLocation, dirName, QStandardPaths::LocateDirectory);
    list << QStandardPaths::locateAll(QStandardPaths::AppConfigLocation, dirName, QStandardPaths::LocateDirectory);

#ifdef Q_OS_UNIX
    if (qEnvironmentVariableIsSet("APPIMAGE"))
        list << makeAbs(QCoreApplication::applicationDirPath() + "/../share/qvmessocket/" + dirName);

    if (qEnvironmentVariableIsSet("SNAP"))
        list << makeAbs(qEnvironmentVariable("SNAP") + "/usr/share/qvmessocket/" + dirName);

    if (qEnvironmentVariableIsSet("XDG_DATA_DIRS"))
        list << makeAbs(qEnvironmentVariable("XDG_DATA_DIRS") + "/" + dirName);

    list << makeAbs("/usr/local/share/qvmessocket/" + dirName);
    list << makeAbs("/usr/local/lib/qvmessocket/" + dirName);
    list << makeAbs("/usr/share/qvmessocket/" + dirName);
    list << makeAbs("/usr/lib/qvmessocket/" + dirName);
    list << makeAbs("/lib/qvmessocket/" + dirName);
#endif

    list.removeDuplicates();
    return list;
}
