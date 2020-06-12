TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS =  qogon_api simple_greeter modeswitcher

qogon_api.depends = qsbp
simple_greeter.depends = qsbp
modeswitcher.depends = qsbp
