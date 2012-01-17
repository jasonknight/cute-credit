#include "receipttemplate.h"
#include <QHashIterator>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDebug>
#include <QStringList>
#include <QDateTime>
ReceiptTemplate::ReceiptTemplate(QObject *parent, QString name) :
    QObject(parent)
{
    QSettings s("JolieRouge","CuteCredit");
    this->m_name = name;

    QString path = s.value("template_path").toString();
    if (path == "") {
        path = "templates";
        s.setValue("template_path",path);
    }
    this->m_path = path;
    QString filename = this->m_path + "/" + this->m_name + ".tmpl";
    QFile f(filename);
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        this->p_template += in.readAll();
        f.close();
    } else {
        qDebug() << "File " << filename << " does not exist, or could not be opened.";
        this->p_template = "";
    }
}
QString ReceiptTemplate::parse(QHash<QString, QString>& key_values) {
    QHashIterator<QString, QString> kv(key_values);
    QString tmp_template = this->p_template;
    while (kv.hasNext()) {
      kv.next();
      QString var = "${" + kv.key() +"}";
      tmp_template = tmp_template.replace(var,this->parseSpecialCodesInLine(kv.value()));
    }
    return this->parseSpecialCodes(tmp_template);
}
QString ReceiptTemplate::parseSpecialCodesInLine(QString l) {
   if (l.indexOf("${sp}") != -1) {
      int s2add = 33 - (l.length() - 5);
      QString spacer = "";
      for (int j = 0; j < s2add; j++)
        spacer += " ";
      l.replace("${sp}",spacer);
    }
    if (l.indexOf("${hr}") != -1) {
     l.replace("${hr}","---------------------------------");
    }
    if (l.indexOf("${datetime}") != -1) {
       QDateTime d = QDateTime::currentDateTime();
       QString dt = d.toString("dd/mm/yyyy hh:mm:ss");
       l.replace("${datetime}",dt);
    }
    if (l.indexOf("${cr}") != -1) {
      l.replace("${cr}","\n");
    }
   return l;
}
QString ReceiptTemplate::parseSpecialCodes(QString tmp_string) {
  QStringList lines = tmp_string.split("\n");
  QString new_tmp;
  for (int i = 0; i < lines.length(); i++) {
    QString l = lines.at(i);
    l = this->parseSpecialCodesInLine(l);
    new_tmp += l + "\n";
  }
  return new_tmp;
}
