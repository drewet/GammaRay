/*
  methodsextension.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2014-2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Anton Kreuzkamp <anton.kreuzkamp@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "methodsextension.h"
#include "objectmethodmodel.h"
#include "propertycontroller.h"
#include "varianthandler.h"
#include "methodargumentmodel.h"
#include "multisignalmapper.h"

#include "common/objectbroker.h"

#include <QMetaMethod>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QTime>

using namespace GammaRay;

MethodsExtension::MethodsExtension(PropertyController* controller) :
  MethodsExtensionInterface(controller->objectBaseName() + ".methodsExtension", controller),
  PropertyControllerExtension(controller->objectBaseName() + ".methods"),
  m_model(new ObjectMethodModel(controller)),
  m_methodLogModel(new QStandardItemModel(this)),
  m_methodArgumentModel(new MethodArgumentModel(this)),
  m_signalMapper(0)
{
  controller->registerModel(m_model, "methods");
  controller->registerModel(m_methodLogModel, "methodLog");
  controller->registerModel(m_methodArgumentModel, "methodArguments");

  ObjectBroker::selectionModel(m_model); // trigger creation
}

MethodsExtension::~MethodsExtension()
{
}

bool MethodsExtension::setQObject(QObject* object)
{
  if (m_object == object)
    return true;
  m_object = object;

  m_model->setMetaObject(object ? object->metaObject() : 0);

  delete m_signalMapper;
  m_signalMapper = new MultiSignalMapper(this);
  connect(m_signalMapper, SIGNAL(signalEmitted(QObject*,int,QVector<QVariant>)), SLOT(signalEmitted(QObject*,int,QVector<QVariant>)));

  if (m_methodLogModel->rowCount() > 0)
    m_methodLogModel->clear();

  setHasObject(true);
  return true;
}

bool MethodsExtension::setMetaObject(const QMetaObject* metaObject)
{
  m_object = 0;
  m_model->setMetaObject(metaObject);
  setHasObject(false);
  return true;
}

void MethodsExtension::signalEmitted(QObject* sender, int signalIndex, const QVector<QVariant>& args)
{
  Q_ASSERT(m_object == sender);

  QStringList prettyArgs;
  foreach (const QVariant &v, args)
    prettyArgs.push_back(VariantHandler::displayString(v));

  m_methodLogModel->appendRow(
  new QStandardItem(tr("%1: Signal %2 emitted, arguments: %3").
  arg(QTime::currentTime().toString("HH:mm:ss.zzz")).
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  arg(sender->metaObject()->method(signalIndex).signature())
#else
  arg(QString(sender->metaObject()->method(signalIndex).methodSignature()))
#endif
  .arg(prettyArgs.join(", "))));
}

void MethodsExtension::activateMethod()
{
  QItemSelectionModel *selectionModel = ObjectBroker::selectionModel(m_model);
  if (selectionModel->selectedRows().size() != 1) {
    return;
  }
  const QModelIndex index = selectionModel->selectedRows().first();

  const QMetaMethod method = index.data(ObjectMethodModelRole::MetaMethod).value<QMetaMethod>();
  m_methodArgumentModel->setMethod(method);
}

void MethodsExtension::connectToSignal()
{
  QItemSelectionModel *selectionModel = ObjectBroker::selectionModel(m_model);
  if (selectionModel->selectedRows().size() != 1) {
    return;
  }
  const QModelIndex index = selectionModel->selectedRows().first();

  const QMetaMethod method = index.data(ObjectMethodModelRole::MetaMethod).value<QMetaMethod>();
  if (method.methodType() == QMetaMethod::Signal) {
    m_signalMapper->connectToSignal(m_object, method);
  }
}

void MethodsExtension::invokeMethod(Qt::ConnectionType connectionType)
{
  if (!m_object) {
      m_methodLogModel->appendRow(
        new QStandardItem(
          tr("%1: Invocation failed: Invalid object, probably got deleted in the meantime.").
          arg(QTime::currentTime().toString("HH:mm:ss.zzz"))));
      return;
  }

  QMetaMethod method;
  QItemSelectionModel *selectionModel = ObjectBroker::selectionModel(m_model);
  if (selectionModel->selectedRows().size() == 1) {
    const QModelIndex index = selectionModel->selectedRows().first();
    method = index.data(ObjectMethodModelRole::MetaMethod).value<QMetaMethod>();
  }

  if (method.methodType() == QMetaMethod::Constructor) {
    m_methodLogModel->appendRow(
      new QStandardItem(
        tr("%1: Invocation failed: Can't invoke constructors.").
        arg(QTime::currentTime().toString("HH:mm:ss.zzz"))));
    return;
  }

  const QVector<MethodArgument> args = m_methodArgumentModel->arguments();
  // TODO retrieve return value and add it to the log in case of success
  // TODO measure executation time and that to the log
  const bool result = method.invoke(m_object.data(), connectionType,
    args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);

  if (!result) {
    m_methodLogModel->appendRow(
      new QStandardItem(
        tr("%1: Invocation failed..").
        arg(QTime::currentTime().toString("HH:mm:ss.zzz"))));
    return;
  }

  m_methodArgumentModel->setMethod(QMetaMethod());
}
