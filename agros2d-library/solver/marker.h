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

#ifndef MARKER_H
#define MARKER_H

#include "util.h"
#include "value.h"

class FieldInfo;

class AGROS_LIBRARY_API Marker
{
public:
    Marker(const FieldInfo *m_fieldInfo, QString m_name);
    virtual ~Marker();

    /// value of one individual variable
    const Value* valueNakedPtr(const QString &name) const;
    const Value* valueNakedPtr(const uint id) const;

    /// value of one individual variable
    const QSharedPointer<Value> value(const QString &name) const;
    const QSharedPointer<Value> value(const uint id) const;

    /// get all values
    const QMap<uint, QSharedPointer<Value> > values() const;

    /// key exists
    bool contains(const QString &name) const;
    const QString valueName(const uint id) const;

    // creates new shared pointer and copy Value inside
    void setValue(const QString &name, Value value);

    // modify existing Value object using its operator=
    void modifyValue(const QString &name, Value value);

    /// return name
    QString name() const { return m_name; }

    /// set name
    void setName(QString paramName) { m_name = paramName; }

    const FieldInfo *fieldInfo() const { return m_fieldInfo; }

    /// returns QString identification of field
    QString fieldId();

    /// ????
    bool evaluate(const QString &name, double time);
    bool evaluate(const uint id, double time);

    /// returns true if all OK
    bool evaluateAllVariables();

    int isNone() const { return m_isNone;}
    void setNone() { m_isNone = true; }

private:
    QString m_name;
    bool m_isNone;

protected:
    const FieldInfo *m_fieldInfo;

private:
    /// variables - the way to customize boundary "template", given by the type parameter
    QMap<uint, QSharedPointer<Value> > m_values;
    QMap<QString, uint> m_valuesHash;

    /// we don't want those objects to be copied since we compare pointers
    Marker(const Marker& );
    Marker& operator =(const Marker& );

};


class Boundary : public Marker
{
public:
    Boundary(const FieldInfo *m_fieldInfo, QString m_name = "", QString m_type = "",
             QMap<QString, Value> m_values = (QMap<QString, Value>()));

    /// get type
    const QString type() const { return m_type; }

    /// set type
    void setType(const QString &type) { m_type = type; }

private:
    /// type of boundary condition, taken from respective module
    QString m_type;


};


class Material : public Marker
{
public:
    Material(const FieldInfo *m_fieldInfo, QString m_name,
             QMap<QString, Value> m_values = (QMap<QString, Value>()));
};


#endif // MARKER_H
