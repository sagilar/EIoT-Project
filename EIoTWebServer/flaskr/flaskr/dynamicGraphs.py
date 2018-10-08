from flask import Flask, render_template, request, session, g, redirect, \
     url_for, abort, flash
import os
import sqlite3
from flask_restful import Resource, Api
from flask_restful import reqparse
import time
import datetime
import csv
import glob
import xlsxwriter
from xlsxwriter.workbook import Workbook
import zipfile
import fnmatch


def getTablaRefrigeradores():
    db = get_db()
    refrigExe = db.execute('select * from refrigeradores')
    refrigFetc = refrigExe.fetchall()
    tablaRefrigeradores=[]
    filaTabla=[]
    for row in refrigFetc:
        for line in row:
            filaTabla.append(line)
        tablaRefrigeradores.append(filaTabla)
        filaTabla=[]
    return tablaRefrigeradores
    
def getTablaEIoT():
    db = get_db()
    eiotExe = db.execute('select * from eiot')
    eiotFetc = eiotExe.fetchall()
    tablaEIoT=[]
    filaTabla=[]
    for row in eiotFetc:
        for line in row:
            filaTabla.append(line)
        tablaEIoT.append(filaTabla)
        filaTabla=[]
    return tablaEIoT

def getTablaRegistros():
    db = get_db()
    tablaEnteraDB=db.execute('select * from records')
    tablaEntera=tablaEnteraDB.fetchall()
    tablaRegistros=[]
    filaTabla=[]
    for vector in tablaEntera:
        for line in vector:
            filaTabla.append(line)
        tablaRegistros.append(filaTabla)
        filaTabla=[]
    return tablaRegistros

def getDispVarEiot():
    db = get_db()
    tablaEnteraDB=db.execute('select distinct iddispositivo from eiot')
    tablaEntera=tablaEnteraDB.fetchall()
    filaTabla=[]
    listaDispStr=[]
    for line in tablaEntera:
        listaDispStr.append(''.join(y for y in line))
    
    listaVarxDisp=[]
    listaVar=[]
    filaVar=[]
    for id in listaDispStr:
        strQuery="select distinct variable from eiot where iddispositivo = '" + id +"'"
        tablaVarDB=db.execute(strQuery)
        tablaVar=tablaVarDB.fetchall()
        for line in tablaVar:
            listaVar.append(''.join(x for x in line))
        listaVarxDisp.append(listaVar)
        filaVar=[]
        listaVar=[]
    tpDisp=tuple(listaDispStr)
    dictEiot=dict(zip(tpDisp, listaVarxDisp))
    
    
    return dictEiot

def getTablesDVEiot(dictionary):
    db = get_db()
    dictContenido={}
    lista=[]
    listaCompleta=[]
    registroCompleto=[]
    for key,value in dictionary.items():
        for item in value:
            strQuery="select valor,fecha from eiot where iddispositivo = '" + key +"' AND variable = '" + item + "'"
            tablaDB=db.execute(strQuery)
            tabla=tablaDB.fetchall()
            for row in tabla:
                for line in row:
                    lista.append(line)
                listaCompleta.append(lista)
                lista=[]
            registroCompleto.append(listaCompleta)
            dictContenido[key+item]=registroCompleto
            listaCompleta=[]
            registroCompleto=[]
    return dictContenido

def crearArchivosEiot():
    db = get_db()
    directory=os.getcwd() + "/flaskr/downloadfiles"
    if not os.path.exists(directory):
        os.makedirs(directory)
    tablaEnteraDB=db.execute('select distinct iddispositivo from eiot')
    tablaEntera=tablaEnteraDB.fetchall()
    filaTabla=[]
    listaDispStr=[]
    for line in tablaEntera:
        listaDispStr.append(''.join(y for y in line))
    
    listaDoc=[]
    lista=[]
    filaVar=[]
    totalFilas=[]
    for id in listaDispStr:
        strQuery="select * from eiot where iddispositivo = '" + id +"'"
        tablaDB=db.execute(strQuery)
        tabla=tablaDB.fetchall()
        for row in tabla:
            for line in row:
                lista.append(line)
            totalFilas.append(lista)
            
            listaDoc=[]
            filaVar=[]
            lista=[]
        with open('flaskr/downloadfiles/CSVEIoTdispositivo-' + id + '.csv', 'w') as myfile:
            wr = csv.writer(myfile, quoting=csv.QUOTE_ALL)
            column_names=[description[0] for description in tablaDB.description]
            wr.writerow(column_names)
            for li in totalFilas:
                wr.writerow(li)
        myfile.close()
        totalFilas=[]
    
    for csvfile in glob.glob(os.path.join('.', 'flaskr/downloadfiles/CSVEIoTdispositivo*.csv')):
        workbook = Workbook((csvfile[:-4] + '.xlsx').replace('CSV','Excel'))
        worksheet = workbook.add_worksheet()
        worksheet.set_column(0,4,30)
        with open(csvfile, 'rt', encoding='utf8') as f:
            reader = csv.reader(f)
            for r, row in enumerate(reader):
                for c, col in enumerate(row):
                    worksheet.write(r, c, col)
                    
        workbook.close()
    
    zipEiotCSV = zipfile.ZipFile('flaskr/static/eiot-csv.zip', 'w', zipfile.ZIP_DEFLATED)
    zipdir('flaskr/downloadfiles/', zipEiotCSV, 'CSVEIoTdispositivo*.csv')
    zipEiotCSV.close()
    
    zipEiotXLSX = zipfile.ZipFile('flaskr/static/eiot-excel.zip', 'w', zipfile.ZIP_DEFLATED)
    zipdir('flaskr/downloadfiles/', zipEiotXLSX, 'ExcelEIoTdispositivo*.xlsx')
    zipEiotXLSX.close()
    return ""

def getDispVarRefrig():
    db = get_db()
    tablaEnteraDB=db.execute('select distinct idnevera from refrigeradores')
    tablaEntera=tablaEnteraDB.fetchall()
    filaTabla=[]
    listaDispStr=[]
    for line in tablaEntera:
        listaDispStr.append(''.join(y for y in line))
    
    listaVarxDisp=[]
    listaVar=[]
    filaVar=[]
    for id in listaDispStr:
        strQuery="select distinct variable from refrigeradores where idnevera = '" + id +"'"
        tablaVarDB=db.execute(strQuery)
        tablaVar=tablaVarDB.fetchall()
        for line in tablaVar:
            listaVar.append(''.join(x for x in line))
        listaVarxDisp.append(listaVar)
        filaVar=[]
        listaVar=[]
    tpDisp=tuple(listaDispStr)
    dictRefrig=dict(zip(tpDisp, listaVarxDisp))
    
    
    return dictRefrig

def getTablesDVRefrig(dictionary):
    db = get_db()
    dictContenido={}
    lista=[]
    listaCompleta=[]
    registroCompleto=[]
    for key,value in dictionary.items():
        for item in value:
            strQuery="select valor,fecha from refrigeradores where idnevera = '" + key +"' AND variable = '" + item + "'"
            tablaDB=db.execute(strQuery)
            tabla=tablaDB.fetchall()
            for row in tabla:
                for line in row:
                    lista.append(line)
                listaCompleta.append(lista)
                lista=[]
            registroCompleto.append(listaCompleta)
            dictContenido[key+item]=registroCompleto
            listaCompleta=[]
            registroCompleto=[]
    return dictContenido

def crearArchivosRefrig():
    db = get_db()
    directory=os.getcwd() + "/flaskr/downloadfiles"
    if not os.path.exists(directory):
        os.makedirs(directory)
    tablaEnteraDB=db.execute('select distinct idnevera from refrigeradores')
    tablaEntera=tablaEnteraDB.fetchall()
    filaTabla=[]
    listaDispStr=[]
    for line in tablaEntera:
        listaDispStr.append(''.join(y for y in line))
    
    listaDoc=[]
    lista=[]
    filaVar=[]
    totalFilas=[]
    for id in listaDispStr:
        strQuery="select * from refrigeradores where idnevera = '" + id +"'"
        tablaDB=db.execute(strQuery)
        tabla=tablaDB.fetchall()
        for row in tabla:
            for line in row:
                lista.append(line)
            totalFilas.append(lista)
            
            listaDoc=[]
            filaVar=[]
            lista=[]
        with open('flaskr/downloadfiles/CSVRefrigerador-' + id + '.csv', 'w') as myfile:
            wr = csv.writer(myfile, quoting=csv.QUOTE_ALL)
            column_names=[description[0] for description in tablaDB.description]
            wr.writerow(column_names)
            for li in totalFilas:
                wr.writerow(li)
        myfile.close()
        totalFilas=[]
    
    for csvfile in glob.glob(os.path.join('.', 'flaskr/downloadfiles/CSVRefrigerador*.csv')):
        workbook = Workbook((csvfile[:-4] + '.xlsx').replace('CSV','Excel'))
        worksheet = workbook.add_worksheet()
        worksheet.set_column(0,4,30)
        with open(csvfile, 'rt', encoding='utf8') as f:
            reader = csv.reader(f)
            for r, row in enumerate(reader):
                for c, col in enumerate(row):
                    worksheet.write(r, c, col)
                    
        workbook.close()
        
    zipRefrigCSV = zipfile.ZipFile('flaskr/static/refrigeradores-csv.zip', 'w', zipfile.ZIP_DEFLATED)
    zipdir('flaskr/downloadfiles/', zipRefrigCSV, 'CSVRefrigerador*.csv')
    zipRefrigCSV.close()
    
    zipRefrigXLSX = zipfile.ZipFile('flaskr/static/refrigeradores-excel.zip', 'w', zipfile.ZIP_DEFLATED)
    zipdir('flaskr/downloadfiles/', zipRefrigXLSX, 'ExcelRefrigerador*.xlsx')
    zipRefrigXLSX.close()
    return ""
    
def zipdir(path, ziph,re):
    # ziph is zipfile handle
    for root, dirs, files in os.walk(path):
        for file in fnmatch.filter(files, re):
            ziph.write(os.path.join(root, file))

def connect_db():
    """Connects to the specific database."""
    rv = sqlite3.connect(app.config['DATABASE'])
    rv.row_factory = sqlite3.Row
    return rv

def get_db():
    """Opens a new database connection if there is none yet for the
    current application context.
    """
    if not hasattr(g, 'sqlite_db'):
        g.sqlite_db = connect_db()
    return g.sqlite_db