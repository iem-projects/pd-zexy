import glob
import os
import re
Import('env prefix')

# generate custom z_zexy files
os.system('test -e src/z_zexy.h && rm src/z_zexy.*')
zexy_src = glob.glob('src/*.c')
os.system('cd src && sh makesource.sh')

zexy = env.SharedLibrary(target = 'zexy', source = zexy_src)
env.Alias('install', env.Install(os.path.join(prefix, 'extra'), zexy))
env.Alias('install', env.Install(os.path.join(prefix, 'doc/5.reference/zexy'), glob.glob('examples/*.pd')))
Default(zexy)
