/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#ifndef SERVERPROPERTIES_H
#define SERVERPROPERTIES_H

#include "common.h"

#include "ui_serverproperties.h"

class ServerPropertiesDlg : public QDialog
{
    Q_OBJECT

public:
    ServerPropertiesDlg(QWidget * parent = 0);
    ~ServerPropertiesDlg();

private:
    void slotAccepted();
    void slotShowMOTDVars(bool checked);
    void slotServerLogToggled(const QModelIndex &index);
private:
    Ui::ServerPropertiesDlg ui;
    ServerProperties m_srvprop = {};
    class ServerLogEventsModel* m_serverlogmodel;
};


#endif
