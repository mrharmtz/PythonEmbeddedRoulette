#run command: python setup.py build --compiler=mingw32
from distutils.core import setup, Extension

moudle_roulette = Extension('roulette', sources = ['roulettemodule.c', 'roulette_module.cpp'])

setup (name = 'roulette',
        version = '1.0',
        description = 'This packge contains the roulette class',
        ext_modules = [moudle_roulette])