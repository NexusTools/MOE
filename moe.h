#ifndef MOE_H
#define MOE_H

#include "ui_moe.h"

class MOE : public QMainWindow, private Ui::MOE
{
    Q_OBJECT
    
public:
    explicit MOE(QWidget *parent = 0);
};

#endif // MOE_H
