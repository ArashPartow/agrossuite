// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef FORM_INTERFACE_H
#define FORM_INTERFACE_H

#include <QtPlugin>

#include "util.h"

class PythonScriptingConsole;

// plugin interface
class AGROS_LIBRARY_API FormInterface : public QDialog
{   
Q_OBJECT
public:
    FormInterface(QWidget *parent = 0) : QDialog(parent) {}
    virtual ~FormInterface() {}

    virtual QString formId() = 0;
    virtual QAction *action() = 0;


public slots:
    virtual int show() = 0;
    virtual void acceptForm() = 0;
    virtual void rejectForm() = 0;

protected:
};

// read forms
void readCustomForms(QMenu *menu);
void readCustomScripts(QMenu *menu, PythonScriptingConsole *console, QWidget *parent);

QT_BEGIN_NAMESPACE
#define FormInterface_IID "org.agros.FormInterface"
Q_DECLARE_INTERFACE(FormInterface,
                    FormInterface_IID)
QT_END_NAMESPACE

#endif // FORM_INTERFACE_H
