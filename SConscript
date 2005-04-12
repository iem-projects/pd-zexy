import glob
import os
import re
Import('env prefix')

# generate custom z_zexy files
os.system('rm z_zexy.* src/z_zexy.*')
zexy_src = glob.glob('src/*.c')
os.system('cd src && sh makesource.sh')

zexy = env.SharedLibrary(target = 'zexy', source = zexy_src)
env.Alias('install', env.Install(os.path.join(prefix, 'extra'), zexy))
Default(zexy)
