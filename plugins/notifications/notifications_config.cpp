/**
 * Copyright 2015 Holger Kaelberer <holger.k@elberer.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "notifications_config.h"
#include "ui_notifications_config.h"
#include "notifyingapplicationmodel.h"

#include <KCModule>
#include <KPluginFactory>

K_PLUGIN_FACTORY(NotificationsConfigFactory, registerPlugin<NotificationsConfig>();)

NotificationsConfig::NotificationsConfig(QWidget *parent, const QVariantList& args)
    : KdeConnectPluginKcm(parent, args, "kdeconnect_notifications_config")
    , m_ui(new Ui::NotificationsConfigUi())
    , appModel(new NotifyingApplicationModel)
{
    qRegisterMetaTypeStreamOperators<NotifyingApplication>("NotifyingApplication");

    m_ui->setupUi(this);
    m_ui->appList->setIconSize(QSize(32,32));

    m_ui->appList->setModel(appModel);

    m_ui->appList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::QHeaderView::Fixed);
    m_ui->appList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::QHeaderView::Stretch);
    m_ui->appList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::QHeaderView::Stretch);
    for (int i = 0; i < 3; i++)
        m_ui->appList->resizeColumnToContents(i);

    connect(m_ui->appList->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            m_ui->appList, SLOT(sortByColumn(int)));

    connect(m_ui->check_persistent, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->spin_urgency, SIGNAL(editingFinished()), this, SLOT(changed()));
    connect(m_ui->check_body, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->check_icons, SIGNAL(toggled(bool)), this, SLOT(changed()));

    connect(appModel, SIGNAL(applicationsChanged()), this, SLOT(changed()));

    connect(config(), SIGNAL(configChanged()), this, SLOT(loadApplications()));
}

NotificationsConfig::~NotificationsConfig()
{
    delete m_ui;
}

void NotificationsConfig::defaults()
{
    KCModule::defaults();
    m_ui->check_persistent->setChecked(false);
    m_ui->spin_urgency->setValue(0);
    m_ui->check_body->setChecked(true);
    m_ui->check_icons->setChecked(true);
    Q_EMIT changed(true);
}

void NotificationsConfig::loadApplications()
{
    appModel->clearApplications();
    QVariantList list = config()->getList("applications");
    for (const auto& a: list) {
        NotifyingApplication app = a.value<NotifyingApplication>();
        if (!appModel->containsApp(app.name)) {
            appModel->appendApp(app);
        }
    }
}

void NotificationsConfig::load()
{
    KCModule::load();
    bool persistent = config()->get("generalPersistent", false);
    m_ui->check_persistent->setChecked(persistent);
    bool body = config()->get("generalIncludeBody", true);
    m_ui->check_body->setChecked(body);
    bool icons = config()->get("generalSynchronizeIcons", true);
    m_ui->check_icons->setChecked(icons);
    int urgency = config()->get("generalUrgency", 0);
    m_ui->spin_urgency->setValue(urgency);

    loadApplications();
    Q_EMIT changed(false);
}

void NotificationsConfig::save()
{
    config()->set("generalPersistent", m_ui->check_persistent->isChecked());
    config()->set("generalIncludeBody", m_ui->check_body->isChecked());
    config()->set("generalSynchronizeIcons", m_ui->check_icons->isChecked());
    config()->set("generalUrgency", m_ui->spin_urgency->value());

    QVariantList list;
    for (const auto& a: appModel->apps())
        list << QVariant::fromValue<NotifyingApplication>(a);
    config()->setList("applications", list);
    KCModule::save();
    Q_EMIT changed(false);
}

#include "notifications_config.moc"
