#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "services/updatechecker.h"

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    m_updateChecker(0),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
    connect(ui->btnReleaseNotes, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleReleaseNotes(bool)));
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

int UpdateDialog::exec(UpdateChecker &uc)
{
    m_updateChecker = &uc;
    return exec();
}

int UpdateDialog::exec()
{
    Q_ASSERT(m_updateChecker);
    connect(m_updateChecker, SIGNAL(receivedResult(int)),
            this, SLOT(slotReceivedUpdateResult(int)));
    if (!m_updateChecker->hasUpdate())
        checkUpdate();
    else
        updateFound();
    int status = QDialog::exec();
    disconnect(m_updateChecker, SIGNAL(receivedResult(int)),
               this, SLOT(slotReceivedUpdateResult(int)));
    return status;
}

void UpdateDialog::checkUpdate()
{
    Q_ASSERT(m_updateChecker);
    ui->btnReleaseNotes->setVisible(false);
    ui->btnReleaseNotes->setChecked(false);
    ui->txtReleaseNotes->setVisible(false);
    ui->lblStatus->setText(tr("Downloading update information..."));
    m_updateChecker->checkUpdate();
    resizeToFit();
}

void UpdateDialog::updateFound()
{
    Q_ASSERT(m_updateChecker);
    ui->btnReleaseNotes->setVisible(true);
    ui->btnReleaseNotes->setChecked(false);
    ui->txtReleaseNotes->setVisible(false);
    ui->lblStatus->setText(get_status());
    ui->txtReleaseNotes->setText(m_updateChecker->releaseNotes());
    resizeToFit();
}

void UpdateDialog::slotReceivedUpdateResult(int result)
{
    Q_ASSERT(m_updateChecker);
    if (m_updateChecker->hasUpdate()) {
        updateFound();
        return;
    }
    QString message;
    switch (result)
    {
    case UpdateChecker::ConnectionError:
        message = tr("Cannot connect to server.");
        break;
    case UpdateChecker::DataError:
        message = tr("Data received from server is incorrect.");
        break;
    case UpdateChecker::UpdateNotFound:
        message = tr("You are already using the latest version of QWinFF.");
        break;
    default:
        message = tr("An unknown error has occurred.");
    }
    ui->lblStatus->setText(QString("%1").arg(message));
    resizeToFit();
}

void UpdateDialog::slotToggleReleaseNotes(bool checked)
{
    // show/hide release notes
    ui->txtReleaseNotes->setVisible(checked);
    resizeToFit();
}

void UpdateDialog::resizeToFit()
{
    resize(width(), ui->lblStatus->height() + ui->btnReleaseNotes->height());
    resize(sizeHint());
}

QString UpdateDialog::get_status()
{
    QStringList result;
    result << tr("A new version of QWinFF has been released!");
    result << "<br>";
    //: %1 is version number, %2 is the project homepage
    result << tr("Version <b>%1</b> is available at %2.")
              .arg(m_updateChecker->versionName(),
                   link(m_updateChecker->downloadPage()));
    QString url = m_updateChecker->downloadUrl();
    if (!url.isEmpty()) {
        result << "<br>";
        result << tr("You can download this version using the link:");
        result << "<br><br>";
        result << link(m_updateChecker->downloadUrl());
    }
    return result.join("");
}

QString UpdateDialog::link(const QString &s)
{
    return QString("<a href=\"%1\">%1</a>").arg(s);
}