#include "konlinejoboutbox.h"
#include "ui_konlinejoboutbox.h"

#include <KMessageBox>

#include "models/models.h"
#include "models/onlinejobmodel.h"

#include "mymoney/mymoneyfile.h"

#include <QDebug>

KOnlineJobOutbox::KOnlineJobOutbox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KOnlineJobOutbox)
{
    ui->setupUi(this);

    onlineJobModel* model = new onlineJobModel(this);
    ui->m_onlineJobView->setModel( model );
    connect(ui->m_buttonSend, SIGNAL( clicked() ), this, SLOT( slotSendJobs() ));
    connect(ui->m_buttonRemove, SIGNAL(clicked()), this, SLOT( slotRemoveJob() ));
    connect(ui->m_buttonEdit, SIGNAL(clicked()), this, SLOT( slotEditJob() ));
    connect(ui->m_onlineJobView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotEditJob(QModelIndex)));
}

KOnlineJobOutbox::~KOnlineJobOutbox()
{
    delete ui;
}

/**
 * @bug Reroduce: create two onlineJobs, select one in outbox, click remove. => exeption is thrown due to roleback.
 * My debugger says ft.commit() calls MyMoneyFileTransaction::roleback() after a successfull commit (confusing, I guess the debbuger is not
 * trustworsthy).
 * Strange: it my system an exeption is thrown. But in main.cpp:184 KMyMoney crashes anyway (-> exeption pointer drangling???)
 */
void KOnlineJobOutbox::slotRemoveJob()
{
  QStringList jobIds;
  QAbstractItemModel* model = ui->m_onlineJobView->model();
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedIndexes();

  foreach(QModelIndex index, indexes) {
    QString jobId = model->data(index, onlineJobModel::OnlineJobId).toString();
    if ( !jobIds.contains(jobId) )
      jobIds.append( jobId );
  }

  if (!jobIds.empty()) {
    MyMoneyFileTransaction ft;
    MyMoneyFile::instance()->removeOnlineJob(jobIds);
    ft.commit();
  }
}

void KOnlineJobOutbox::slotSendJobs()
{
  QList<onlineJob> list;
  foreach (const onlineJob job, MyMoneyFile::instance()->onlineJobList())
    list.append(job);

  qDebug() << "I shall send " << list.count() << " onlineJobs";
  emit sendJobs( list );
}

void KOnlineJobOutbox::slotEditJob()
{
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedIndexes();
  QString jobId = ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobId).toString();
  Q_ASSERT( !jobId.isEmpty() );
  emit editJob(jobId);
}

void KOnlineJobOutbox::slotEditJob(const QModelIndex &index)
{
  QString jobId = ui->m_onlineJobView->model()->data(index, onlineJobModel::OnlineJobId).toString();
  emit editJob(jobId);
}
