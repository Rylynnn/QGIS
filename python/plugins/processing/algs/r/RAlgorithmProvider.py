# -*- coding: utf-8 -*-

"""
***************************************************************************
    RAlgorithmProvider.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.core import QgsApplication
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.core.ProcessingLog import ProcessingLog
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.gui.EditScriptAction import EditScriptAction
from processing.gui.DeleteScriptAction import DeleteScriptAction
from processing.gui.CreateNewScriptAction import CreateNewScriptAction
from processing.script.WrongScriptException import WrongScriptException
from processing.gui.GetScriptsAndModels import GetRScriptsAction
from processing.tools.system import isWindows

from .RUtils import RUtils
from .RAlgorithm import RAlgorithm

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class RAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        super().__init__()
        self.activate = False
        self.actions.append(CreateNewScriptAction(
            'Create new R script', CreateNewScriptAction.SCRIPT_R))
        self.actions.append(GetRScriptsAction())
        self.contextMenuActions = \
            [EditScriptAction(EditScriptAction.SCRIPT_R),
             DeleteScriptAction(DeleteScriptAction.SCRIPT_R)]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(
            self.name(), RUtils.RSCRIPTS_FOLDER,
            self.tr('R Scripts folder'), RUtils.defaultRScriptsFolder(),
            valuetype=Setting.MULTIPLE_FOLDERS))
        if isWindows():
            ProcessingConfig.addSetting(Setting(
                self.name(),
                RUtils.R_FOLDER, self.tr('R folder'), RUtils.RFolder(),
                valuetype=Setting.FOLDER))
            ProcessingConfig.addSetting(Setting(
                self.name(),
                RUtils.R_LIBS_USER, self.tr('R user library folder'),
                RUtils.RLibs(), valuetype=Setting.FOLDER))
            ProcessingConfig.addSetting(Setting(
                self.name(),
                RUtils.R_USE64, self.tr('Use 64 bit version'), False))

    def unload(self):
        AlgorithmProvider.unload(self)
        ProcessingConfig.removeSetting(RUtils.RSCRIPTS_FOLDER)
        if isWindows():
            ProcessingConfig.removeSetting(RUtils.R_FOLDER)
            ProcessingConfig.removeSetting(RUtils.R_LIBS_USER)
            ProcessingConfig.removeSetting(RUtils.R_USE64)

    def icon(self):
        return QgsApplication.getThemeIcon("/providerR.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerR.svg")

    def name(self):
        return 'R scripts'

    def id(self):
        return 'r'

    def _loadAlgorithms(self):
        folders = RUtils.RScriptsFolders()
        self.algs = []
        for f in folders:
            self.loadFromFolder(f)

        folder = os.path.join(os.path.dirname(__file__), 'scripts')
        self.loadFromFolder(folder)

    def loadFromFolder(self, folder):
        if not os.path.exists(folder):
            return
        for path, subdirs, files in os.walk(folder):
            for descriptionFile in files:
                if descriptionFile.endswith('rsx'):
                    try:
                        fullpath = os.path.join(path, descriptionFile)
                        alg = RAlgorithm(fullpath)
                        if alg.name.strip() != '':
                            self.algs.append(alg)
                    except WrongScriptException as e:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, e.msg)
                    except Exception as e:
                        ProcessingLog.addToLog(
                            ProcessingLog.LOG_ERROR,
                            self.tr('Could not load R script: %s\n%s' % (descriptionFile, str(e))))
