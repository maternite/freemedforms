# include general workbench
include(../buildspecs/config.pri)

win32 {
    DLLDESTDIR = $${BUILD_BINARY_PATH}
}

include(rpath.pri)

CONFIG *= dll

DESTDIR  = $${BUILD_LIB_PATH}


# TARGET = $$qtLibraryTarget($$TARGET)
