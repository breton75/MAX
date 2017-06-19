#ifndef SV_FNT
#define SV_FNT

#include <QString>
#include <QDateTime>
#include <QDir>

namespace svfnt {

const QString DATE_FORMAT1 = "ddMMyyyy";
const QString TIME_FORMAT1 = "hhmmss";

QString replace_re(QDateTime &dt, QString &extention, QString &string)
{
    return string.replace("{EXT}", extention)
                 .replace("{DATE}", dt.date().toString(DATE_FORMAT1))
                 .replace("{TIME}", dt.time().toString(TIME_FORMAT1));
}

QString get_folder_name(QDateTime &dt, QString &extention, QString &string)
{

        QString workdir = replace_re(dt, extention, string);

        QDir dir(QDir::currentPath());

        if(!dir.mkpath(workdir))
            return "";

        dir.setPath(workdir);

        QString path = QDir::toNativeSeparators(dir.absolutePath());
        path += QDir::separator();

        return path;

//        workdir = os.path.abspath(workdir)
//        workdir = workdir.replace("\u005c", "/")
//                if workdir[-1] != '/': workdir += '/'
//        if not os.path.exists(workdir):
//            os.makedirs(workdir)

//        return workdir

}

//def get_path(config, extention, **kwargs):
//    try:
//        if not (c_workdir in config or c_filename_template in config):
//            raise Exception('config must contains params "filename_template" and "workdir" specifing path and names of created files\nexample: filename_template = home/user/  workdir=test_main')

//        if len(config[c_workdir]) == 0:
//            raise Exception("Work directory not specified")

//        workdir = ''
//        if ('only_filename' not in kwargs) or (bool(kwargs['only_filename']) == False):
//            workdir = get_folder_name(config, extention)

//        filename = replace_re(config, extention, config[c_filename_template])

//        return workdir + filename + '.' + extention


//    except Exception as E:
//        print('error in func get_path(): %s' % E, file=sys.stderr)
//        return None
}


#endif // SV_FNT

