/***************************************************************************
                             kgpgfile.cpp
                             -------------------
    begin                : Fri Jan 23 2004
    copyright            : (C) 2004,2005 by Thomas Baumgart
    email                : thb@net-bembel.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config-kmymoney.h>



// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>
#include <qdir.h>
#include <qstring.h>

#include <qeventloop.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3ValueList>


// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <klocale.h>
#include <k3process.h>
#include <k3passworddialog.h>
#include <klibloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kgpgfile.h"

#if 0
class KGPGFileFactory : public KLibFactory
{
public:
    KGPGFileFactory() : KLibFactory() {}
    ~KGPGFileFactory(){}
    QObject *createObject( QObject *, const char *, const char*, const QStringList & )
    {
        return new KGPGFile;
    }
};

extern "C" {
    void *init_libkgpgfile()
    {
        return new KGPGFileFactory;
    }
}
#endif

KGPGFile::KGPGFile(const QString& fn, const QString& homedir, const QString& options) :
  m_options(options),
  m_homedir(homedir),
  m_readRemain(0),
  m_needExitLoop(false)
{
  setName(fn);
  m_exitStatus = -2;
  m_comment = "created by KGPGFile";
  // qDebug("ungetchbuffer %d", m_ungetchBuffer.length());
}

KGPGFile::~KGPGFile()
{
  close();
}

void KGPGFile::init(void)
{
//FIXME:Port to Qt4
//setFlags(IO_Sequential);
//setStatus(IO_Ok);
//setState(0);
}

void KGPGFile::setName(const QString& fn)
{
  m_fn = fn;
  if(!fn.isEmpty() && fn[0] == '~') {
    m_fn = QDir::homePath()+fn.mid(1);

  } else if(QDir::isRelativePath(m_fn)) {
    QDir dir(fn);
    m_fn = dir.absolutePath();
  }
  // qDebug("setName: '%s'", m_fn.data());
}

void KGPGFile::flush(void)
{
  // no functionality
}

void KGPGFile::addRecipient(const Q3CString& recipient)
{
  m_recipient << recipient;
}

bool KGPGFile::open(int mode)
{
  return open(mode, QString(), false);
}

bool KGPGFile::open(int mode, const QString& cmdArgs, bool skipPasswd)
{
  bool useOwnPassphrase = (getenv("GPG_AGENT_INFO") == 0);

  // qDebug("KGPGFile::open(%d)", mode);
  m_errmsg.resize(1);
  if(isOpen()) {
    // qDebug("File already open");
    return false;
  }

  // qDebug("check filename empty");
  if(m_fn.isEmpty())
    return false;

  // qDebug("setup file structures");
  init();

//FIXME:Port to Qt4
  //setMode(mode);

  // qDebug("check valid access mode");
  if(!(isReadable() || isWritable()))
    return false;

  if(isWritable()) {
    // qDebug("check recipient count");
    if(m_recipient.count() == 0)
      return false;
    // qDebug("check access rights");
    //FIXME:Port to Qt4
    //if(!checkAccess(m_fn, W_OK))
    //  return false;
  }

  QStringList args;
  if(cmdArgs.isEmpty()) {
    args << "--homedir" << QString("\"%1\"").arg(m_homedir)
        << "-q"
        << "--batch";

    if(isWritable()) {
      args << "-ea"
          << "-z" << "6"
          << "--comment" << QString("\"%1\"").arg(m_comment)
          << "--trust-model=always"
          << "-o" << QString("\"%1\"").arg(m_fn);
      QLinkedList<QByteArray>::Iterator it;
      for(it = m_recipient.begin(); it != m_recipient.end(); ++it)
        args << "-r" << QString("\"%1\"").arg(QString(*it));

      // some versions of GPG had trouble to replace a file
      // so we delete it first
      QFile::remove(m_fn);
    } else {
      args << "-da";
      if(useOwnPassphrase)
        args << "--passphrase-fd" << "0";
      else
        args << "--use-agent";
      args << "--no-default-recipient" << QString("\"%1\"").arg(m_fn);
    }
  } else {
    args = QStringList::split(" ", cmdArgs);
  }

  Q3CString pwd;
  if(isReadable() && useOwnPassphrase && !skipPasswd) {
    K3PasswordDialog dlg(K3PasswordDialog::Password,false,0);
    dlg.setPrompt(i18n("Enter passphrase"));
    dlg.addLine(i18n("File"), m_fn);
    dlg.adjustSize();
    if (dlg.exec() == QDialog::Rejected)
      return false;
    pwd = Q3CString(dlg.password());
  }

  // qDebug("starting GPG process");
  if(!startProcess(args))
    return false;

  // qDebug("check GPG process running");
  if(!m_process) {
    // if the process is not present anymore, we have to check
    // if it was a read operation and we might already have data
    // and the process finished normally. In that case, we
    // just continue.
    if(isReadable()) {
      if(m_ungetchBuffer.isEmpty())
        return false;
    } else
      return false;
  }

  if(isReadable() && useOwnPassphrase && !skipPasswd) {
    // qDebug("Passphrase is '%s'", pwd.data());
    if(_write(pwd.data(), pwd.length()) == -1) {
      // qDebug("Sending passphrase failed");
      return false;
    }
    m_process->closeStdin();
  }

//FIXME: Port to Qt4
//  setState( IO_Open );
//  ioIndex = 0;
  // qDebug("File open");
  return true;
}

bool KGPGFile::startProcess(const QStringList& args)
{
  // now start the K3Process with GPG
  m_process = new K3ShellProcess();
  *m_process << "gpg";
  *m_process << args;

  // QString arglist = args.join(":");
  // qDebug("gpg '%s'", arglist.data());

  connect(m_process, SIGNAL(processExited(K3Process *)),
          this, SLOT(slotGPGExited(K3Process *)));

  connect(m_process, SIGNAL(receivedStdout(K3Process*, char*, int)),
          this, SLOT(slotDataFromGPG(K3Process*, char*, int)));

  connect(m_process, SIGNAL(receivedStderr(K3Process*, char*, int)),
          this, SLOT(slotErrorFromGPG(K3Process*, char*, int)));

  connect(m_process, SIGNAL(wroteStdin(K3Process *)),
          this, SLOT(slotSendDataToGPG(K3Process *)));

  if(!m_process->start(K3Process::NotifyOnExit, (K3Process::Communication)(K3Process::Stdin|K3Process::Stdout|K3Process::Stderr))) {
    // qDebug("m_process->start failed");
    delete m_process;
    m_process = 0;
    return false;
  }

  // let the process settle and see if it starts and survives ;-)
  kapp->processEvents(QEventLoop::AllEvents, 100);
  return true;
}

void KGPGFile::close(void)
{
  // qDebug("KGPGFile::close()");
  if(!isOpen()) {
    // qDebug("File not open");
    return;
  }

  // finish the K3Process and clean up things
  if(m_process) {
    if(isWritable()) {
      // qDebug("Finish writing");
      if(m_process->isRunning()) {
        m_process->closeStdin();
        // now wait for GPG to finish
        m_needExitLoop = true;
        qApp->enter_loop();
      } else
        m_process->kill();

    } else if(isReadable()) {
      // qDebug("Finish reading");
      if(m_process->isRunning()) {
        m_process->closeStdout();
        // now wait for GPG to finish
        m_needExitLoop = true;
        qApp->enter_loop();
      } else
        m_process->kill();
    }
  }
  m_ungetchBuffer = Q3CString();

//FIXME: Port to Qt4
//  setState(0);
  m_recipient.clear();
  // qDebug("File closed");
}

int KGPGFile::getch(void)
{
  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  int ch;

  if(!m_ungetchBuffer.isEmpty()) {
    ch = (m_ungetchBuffer)[0] & 0xff;
    m_ungetchBuffer.remove(0, 1);

  } else {
    char buf[1];
    ch = (read(buf,1) == 1) ? (buf[0] & 0xff) : EOF;
  }

  // qDebug("getch returns 0x%02X", ch);
  return ch;
}

int KGPGFile::ungetch(int ch)
{
  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  if(ch != EOF) {
    // qDebug("store 0x%02X in ungetchbuffer", ch & 0xff);
    m_ungetchBuffer.insert(0, ch & 0xff);
  }

  return ch;
}

int KGPGFile::putch(int c)
{
  char  buf[1];
  buf[0] = c;
  if(write(buf, 1) != EOF)
    return c;
  return EOF;
}

Q_LONG KGPGFile::write(const char *data, Q_ULONG maxlen)
{
  if(!isOpen())
    return EOF;
  if(!isWritable())
    return EOF;

  return _write(data, maxlen);
}

Q_LONG KGPGFile::_write(const char *data, Q_ULONG maxlen)
{
  if(!m_process)
    return EOF;
  if(!m_process->isRunning())
    return EOF;

  if(m_process->writeStdin(data, maxlen)) {
    // wait until the data has been written
    m_needExitLoop = true;
    qApp->enter_loop();
    if(!m_process)
      return EOF;
    return maxlen;

  } else
    return EOF;
}

Q_LONG KGPGFile::read(char *data, Q_ULONG maxlen)
{
  // char *oridata = data;
  if(maxlen == 0)
    return 0;

  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  Q_ULONG nread = 0;
  if(!m_ungetchBuffer.isEmpty()) {
    unsigned l = m_ungetchBuffer.length();
    if(maxlen < l)
      l = maxlen;
    memcpy(data, m_ungetchBuffer, l);
    nread += l;
    data = &data[l];
    m_ungetchBuffer.remove(0, l);

    if(!m_process) {
      // qDebug("read %d bytes from unget buffer", nread);
      // dumpBuffer(oridata, nread);
      return nread;
    }
  }

  // check for EOF
  if(!m_process) {
    // qDebug("EOF (no process)");
    return EOF;
  }

  m_readRemain = maxlen - nread;
  m_ptrRemain = data;
  if(m_readRemain) {
    m_process->resume();
    m_needExitLoop = true;
    qApp->enter_loop();
  }
  // if nothing has been read (maxlen-m_readRemain == 0) then we assume EOF
  if((maxlen - m_readRemain) == 0) {
    // qDebug("EOF (nothing read)");
    return EOF;
  }
  // qDebug("return %d bytes", maxlen - m_readRemain);
  // dumpBuffer(oridata, maxlen - m_readRemain);
  return maxlen - m_readRemain;
}

QByteArray KGPGFile::readAll(void)
{
  // use a larger blocksize than in the QIODevice version
  const int blocksize = 8192;
  int nread = 0;
  QByteArray ba;
  while ( !atEnd() ) {
    ba.resize( nread + blocksize );
    int r = read( ba.data()+nread, blocksize );
    if ( r < 0 )
      return QByteArray();
    nread += r;
  }
  ba.resize( nread );
  return ba;
}

void KGPGFile::slotGPGExited(K3Process* )
{
  // qDebug("GPG finished");
  if(m_process) {
    if(m_process->normalExit()) {
      m_exitStatus = m_process->exitStatus();
      if(m_exitStatus != 0)
        {
//FIXME: Port to Qt4
//        setStatus(IO_UnspecifiedError);
        }
    } else {
      m_exitStatus = -1;
    }
    delete m_process;
    m_process = 0;
  }

  if(m_needExitLoop) {
    m_needExitLoop = false;
    qApp->exit_loop();
  }
}

void KGPGFile::slotDataFromGPG(K3Process* proc, char* buf, int len)
{
  // qDebug("Received %d bytes on stdout", len);

  // copy current buffer to application
  int copylen;
  copylen = m_readRemain < len ? m_readRemain : len;
  if(copylen != 0) {
    memcpy(m_ptrRemain, buf, copylen);
    m_ptrRemain += copylen;
    buf += copylen;
    m_readRemain -= copylen;
    len -= copylen;
  }

  // store rest of buffer in ungetch buffer
  while(len--) {
    m_ungetchBuffer += *buf++;
  }

  // if we have all the data the app requested, we can safely suspend
  if(m_readRemain == 0) {
    proc->suspend();
    // wake up the recipient
    if(m_needExitLoop) {
      m_needExitLoop = false;
      qApp->exit_loop();
    }
  }
  // qDebug("end slotDataFromGPG");
}

void KGPGFile::slotErrorFromGPG(K3Process *, char *buf, int len)
{
  // qDebug("Received %d bytes on stderr", len);
  Q3CString msg;
  msg.setRawData(buf, len);
  m_errmsg += msg;
  msg.resetRawData(buf, len);
}

void KGPGFile::slotSendDataToGPG(K3Process *)
{
  // qDebug("wrote stdin");
  if(m_needExitLoop) {
    m_needExitLoop = false;
    qApp->exit_loop();
  }
}

bool KGPGFile::GPGAvailable(void)
{
  QString output;
  char  buffer[1024];
  Q_LONG len;

  KGPGFile file;
  file.open(QIODevice::ReadOnly, "--version", true);
  while((len = file.read(buffer, sizeof(buffer)-1)) != EOF) {
    buffer[len] = 0;
    output += QString(buffer);
  }
  file.close();
  return !output.isEmpty();
}

bool KGPGFile::keyAvailable(const QString& name)
{
  QString output;
  char  buffer[1024];
  Q_LONG len;

  KGPGFile file;
  QString args = QString("--list-keys --list-options no-show-photos %1").arg(name);
  file.open(QIODevice::ReadOnly, args, true);
  while((len = file.read(buffer, sizeof(buffer)-1)) != EOF) {
    buffer[len] = 0;
    output += QString(buffer);
  }
  file.close();
  return !output.isEmpty();
}

void KGPGFile::publicKeyList(QStringList& list)
{
  QMap<QString, QString> map;
  QString output;
  char  buffer[1024];
  Q_LONG len;

  list.clear();
  KGPGFile file;
  file.open(QIODevice::ReadOnly, "--list-keys --with-colons", true);
  while((len = file.read(buffer, sizeof(buffer)-1)) != EOF) {
    buffer[len] = 0;
    output += QString(buffer);
  }
  file.close();

  // now parse the data. it looks like:
  /*
    tru::0:1210616414:1214841688:3:1:5
    pub:u:1024:17:9C59DB40B75DD3BA:2001-06-23:::u:Thomas Baumgart <thomas.baumgart@syrocon.de>::scaESCA:
    uid:u::::2001-11-29::63493BF182C494227E198FE5DA00ACDF63961AFB::Thomas Baumgart <thb@net-bembel.de>:
    uid:u::::2001-11-29::00A393737BC120C98A6402B921599F6D72058DD8::Thomas Baumgart <ipwizard@users.sourceforge.net>:
    sub:u:1024:16:85968A70D1F83C2B:2001-06-23::::::e:
  */
  QStringList lines = QStringList::split("\n", output);
  QStringList::iterator it;
  QString currentKey;
  for(it = lines.begin(); it != lines.end(); ++it) {
    // qDebug("Parsing: '%s'", (*it).data());
    QStringList fields = QStringList::split(":", (*it), true);
    QString val;
    if(fields[0] == "pub") {
      currentKey = fields[4];
      val = QString("%1:%2").arg(currentKey).arg(fields[9]);
      map[val] = val;
    } else if(fields[0] == "uid") {
      val = QString("%1:%2").arg(currentKey).arg(fields[9]);
      map[val] = val;
    }
  }
  list = map.values();
}


void KGPGFile::secretKeyList(QStringList& list)
{
  QString output;
  char  buffer[1024];
  Q_LONG len;

  list.clear();
  KGPGFile file;
  file.open(QIODevice::ReadOnly, "--list-secret-keys --with-colons", true);
  while((len = file.read(buffer, sizeof(buffer)-1)) != EOF) {
    buffer[len] = 0;
    output += QString(buffer);
  }
  file.close();

  // now parse the data. it looks like:
  /*
    sec::1024:17:9C59DB40B75DD3BA:2001-06-23::::Thomas Baumgart <ipwizard@users.sourceforge.net>:::
    uid:::::::::Thomas Baumgart <thb@net-bembel.de>:
    ssb::1024:16:85968A70D1F83C2B:2001-06-23:::::::
    sec::1024:17:59B0F826D2B08440:2005-01-03:2010-01-02:::KMyMoney emergency data recovery <kmymoney-recover@users.sourceforge.net>:::
    ssb::2048:16:B3DABDC48C0FE2F3:2005-01-03:::::::
  */
  QStringList lines = QStringList::split("\n", output);
  QStringList::iterator it;
  QString currentKey;
  for(it = lines.begin(); it != lines.end(); ++it) {
    // qDebug("Parsing: '%s'", (*it).data());
    QStringList fields = QStringList::split(":", (*it), true);
    if(fields[0] == "sec") {
      currentKey = fields[4];
      list << QString("%1:%2").arg(currentKey).arg(fields[9]);
    } else if(fields[0] == "uid") {
      list << QString("%1:%2").arg(currentKey).arg(fields[9]);
    }
  }
}

/*
// key generation
  char * gpg_input =
    g_strdup_printf("Key-Type: DSA\n"
                    "Key-Length: 1024\n"
                    "Subkey-Type: ELG-E\n"
                    "Subkey-Length: 1024\n"
                    "Name-Real: %s\n"
                    "Name-Comment: %s\n"
                    "Name-Email: %s\n"
                    "Passphrase: %s\n"
                    "%%commit\n",
                    username ? username : "",
                    idstring ? idstring : "",
                    email ? email : "",
                    passphrase ? passphrase : "");
  char * argv [] =
  { "gpg",
    "--batch",
    "-q",
    "--gen-key",
    "--keyring",
    "~/.gnucash/gnucash.pub",
    "--secret-keyring",
    "~/.gnucash/gnucash.sec",
    NULL
  };

  char * retval = gnc_gpg_transform(gpg_input, strlen(gpg_input), NULL, argv);
  g_free(gpg_input);
  return retval;

 */

#if KMM_DEBUG
void KGPGFile::dumpBuffer(char *s, int len) const
{
  QString data, tmp, chars;
  unsigned long addr = 0x0;

  while(1) {
    if(addr && !(addr & 0x0f)) {
      qDebug("%s %s", qPrintable(data), qPrintable(chars));
      if(!len)
        break;
    }
    if(!(addr & 0x0f)) {
      data = tmp.sprintf("%08lX", addr);
      chars = QString();
    }
    if(!(addr & 0x03)) {
      data += " ";
    }
    ++addr;

    if(!len) {
      data += "  ";
      chars += " ";
      continue;
    }

    data += tmp.sprintf("%02X", *s & 0xff);
    if(*s >= ' ' && *s <= '~')
      chars += *s & 0xff;
    else
      chars += '.';
    ++s;
    --len;
  }
}
#endif

#include "kgpgfile.moc"
