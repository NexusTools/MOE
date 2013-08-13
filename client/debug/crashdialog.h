#ifndef CRASHDIALOG_H
#define CRASHDIALOG_H

#include "../core/moeengine.h"

#include <QDialog>

namespace Ui {
class CrashDialog;
}

class CrashDialog : public QDialog
{
    Q_OBJECT
    
public:
   static inline void init(MoeClientEngine* engine, bool quitOnClose =true) {
        new CrashDialog(engine, quitOnClose);
   }
    
public slots:
    void showError(QString);
    void setWrapError(bool);

private:
    explicit CrashDialog(MoeClientEngine* engine, bool quitOnClose);
    ~CrashDialog();

    Ui::CrashDialog *ui;
};

#endif // CRASHDIALOG_H
