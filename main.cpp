#include <QCoreApplication>
#include <QPaintEngine>
#include <QImage>
#include <QRect>
#include <QPainter>
#include <QDebug>
#include <QMetaObject>
#include <QObject>
#include <QMetaMethod>
#include <QMetaProperty>

#include <cstdio>
#include <unistd.h>

#include <dlfcn.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif



void dump_qtClass(void* ptr) {
  printf("INSTANCE ADDRESS: 0x%lx\n", ptr);
  QObject *object = static_cast<QObject *>((QObject*) ptr);
  const QMetaObject *meta = object->metaObject();
  qDebug() << meta->className();

  qDebug() << "Methods";
  for (int id = 0; id < meta->methodCount(); ++id) {
    const QMetaMethod method = meta->method(id);
    if (method.methodType() == QMetaMethod::Slot && method.access() == QMetaMethod::Public)
      qDebug() << method.access() << method.name() << method.parameterNames() << method.parameterTypes();
  }

  qDebug() << "Properties";
  for (int id = 0; id < meta->propertyCount(); ++id) {
    const QMetaProperty property = meta->property(id);
    qDebug() << property.name() << property.type();
  }

  qDebug() << "Enumerators";
  for (int id = 0; id < meta->enumeratorCount(); ++id) {
    const QMetaEnum en = meta->enumerator(id);
    qDebug() << en.name();
    for (int j = 0; j < en.keyCount(); j++) {
      qDebug() << en.key(j);
    }
    qDebug() << "";
  }
}

uint64_t sendUpdateAddress = 0x00794330;
uint64_t getInstanceAddress = 0x00792ac0;

uint64_t *(*getInstance)(void) = (uint64_t * (*)(void)) getInstanceAddress;
void (*sendUpdate)(void*, ...) = (void (*)(void*,...)) sendUpdateAddress;


int main(int argc, char **argv, char **envp) {
	qputenv("QMLSCENE_DEVICE", "epaper");
	qputenv("QT_QPA_PLATFORM", "epaper:enable_fonts");

	QCoreApplication a(argc, argv);

	puts("Running on Paper Pro!!\n");

	auto ptr = getInstance();
	auto instance = reinterpret_cast<QObject *>(ptr);

	printf("Got instance: 0x%llx\n", ptr);

	QRect rect(100, 100, 900, 900);

	dump_qtClass(instance);

	sleep(3);
	puts("Clearing screen\n");
	QMetaObject::invokeMethod(instance, "clearScreen", Qt::DirectConnection); //no such method :(

	sleep(3);
	puts("Sending update...\n");
	sendUpdate(ptr, rect, 0, 3, 1); //this flashes screen

	sleep(1);
	sendUpdate(ptr, rect, 0, 0, 0); //this does nothing (no error too)

	return a.exec();
}

extern "C" {
int __libc_start_main(int (*_main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end)) {

	puts("HOOKING LIBC_START\n");

	typeof(&__libc_start_main) hook_func = (typeof(&__libc_start_main)) dlsym(RTLD_NEXT, "__libc_start_main");

	return hook_func(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

}
