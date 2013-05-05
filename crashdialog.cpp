#include "crashdialog.h"
#include "ui_crashdialog.h"



CrashDialog::CrashDialog(MoeEngine* engine, bool quitOnClose) :
    ui(new Ui::CrashDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted()), engine, SLOT(start()), Qt::DirectConnection);

    if(quitOnClose)
        connect(this, SIGNAL(rejected()), qApp, SLOT(quit()));

    connect(engine, SIGNAL(destroyed()), this, SLOT(deleteLater()), Qt::QueuedConnection);
    connect(engine, SIGNAL(crashed(QString)), this, SLOT(showError(QString)), Qt::QueuedConnection);
}

void CrashDialog::showError(QString err) {
    ui->textBrowser->setPlainText(err);
    exec();
}

CrashDialog::~CrashDialog()
{
    delete ui;
}
