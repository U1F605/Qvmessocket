#pragma once

#include <QMap>
#include <QObject>
#include <QStringList>

namespace Qvmessocket::ui::styles
{
    struct QvStyle
    {
        enum StyleType
        {
            QVSTYLE_FACTORY,
            QVSTYLE_QSS
        } Type;
        QString Name;
        QString qssPath;
    };

    class QvStyleManager : QObject
    {
      public:
        QvStyleManager(QObject *parent = nullptr);
        inline QStringList AllStyles() const
        {
            return styles.keys();
        }
        bool ApplyStyle(const QString &);

      private:
        void ReloadStyles();
        QMap<QString, QvStyle> styles;
    };

    inline QvStyleManager *StyleManager = nullptr;
}

using namespace Qvmessocket::ui::styles;
