#run command: python setup.py build --compiler=mingw32
from distutils.core import setup, Extension

roulette = Extension('roulette', sources=['roulette_module.cpp'])

setup(name='roulette'
      , version='1.0 beta'
      , description='roulette module'
      , author='mrharmtz'
      , ext_modules=[roulette])
