#Se definen las librerías a utilizar
from flask import Flask, render_template, request, jsonify, session, g, redirect, \
     url_for, abort, flash
import os
import sqlite3
from flask_restful import Resource, Api
from flask_restful import reqparse
import time
import datetime
from flaskr import dynamicGraphs


#Se inicializa la aplicación

app = Flask(__name__)
app.config.from_object(__name__) # load config from this file
api = Api(app)

app.config.update(dict(
    DATABASE=os.path.join(app.root_path, 'flaskr.db'),
    SECRET_KEY='development key',
    USERNAME='admin',
    PASSWORD='adminpass'
))
app.config.from_envvar('FLASKR_SETTINGS', silent=True)


#Función para obtener el tiempo en formato timestamp
def timestamp_string():
    ts = time.time()
    st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
    return st

def abort_if_record_doesnt_exist(record_id):
    db = get_db()
    registros = db.execute('select * from records')
    salida = registros.fetchall()
    
    if record_id not in salida:
        abort(404, message="El registro {} no existe".format(record_id))
    return ""

parser = reqparse.RequestParser()
parser.add_argument('id')
parser.add_argument('id_disp')
parser.add_argument('id_refrig')
parser.add_argument('var')
parser.add_argument('valor')


#######
#Se definen las clases para manejar las APIs de la tabla registros
#Class Var:Protocolos GET, DELETE Y PUT (SELECT, DELETE Y UPDATE) para tabla registros.
class Var(Resource):
    def get(self, record_id):
        #abort_if_record_doesnt_exist(record_id)
        db = get_db()
        registro = db.execute('select valor, fecha from records where registro = ?',record_id)
        salida = registro.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row]) 
        return data,201
    
    def delete(self, record_id):
        #abort_if_record_doesnt_exist(record_id)
        db = get_db()
        registros = db.execute('delete from records where registro = ?',record_id)
        salida = registros.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row])
        return data,201
    
    def put(self,record_id):
        #abort_if_record_doesnt_exist(record_id)
        db = get_db()
        args = parser.parse_args()
        fechaStr=timestamp_string()
        valor = args['valor']
        upd=db.execute('update records set valor = ?, fecha = ? where registro = ?',[valor,fechaStr,record_id])
        db.commit()
        result=upd.fetchall()
        data=[]
        for row in result:
            data.append([x for x in row])
        return data,201

#Class VarQuery:Dispone la API GET con selección de parámetro en la URL (?id="No.ID") Tabla registros
class VarQuery(Resource):
    def get(self, record_id=None):
        #abort_if_record_doesnt_exist(record_id)
        db = get_db()
        query_parameters = request.args
        if 'id' in request.args:
            id = request.args['id']
            registro = db.execute('select valor, fecha from records where registro = ?',id)
        else:
            registro = db.execute('select * from records')
        
        
        salida = registro.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row]) 
        return data,201
    
#Class VarList: Dispone las API para leer todos los registros y añadir nuevo registro (PUT=INSERT)
class VarList(Resource):
    def get(self):
        db = get_db()
        registros = db.execute('select * from records')
        salida = registros.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row])
        return data,201

    def post(self):
        db = get_db()
        args = parser.parse_args()
        valor=args['valor']
        fechaStr=timestamp_string()
        print('Valor: ' + valor + ' fecha ' + fechaStr)
        
        req = db.execute('insert into records(valor,fecha) values (?,?)',[valor,fechaStr])
        db.commit()
        result=req.fetchall()
        
        data=[]
        for row in result:
            data.append([x for x in row])
        return data, 201

#######
#Se definen las clases para manejar las APIs de la tabla refrigeradores
#Class Refrig:Protocolos GET, DELETE Y PUT (SELECT, DELETE Y UPDATE) para tabla refrigeradores.
class Refrig(Resource):
    def get(self, record_id):
        #abort_if_record_doesnt_exist(record_id)
        db = get_db()
        registro = db.execute('select idnevera, variable, valor, fecha from refrigeradores where registro = ?',record_id)
        salida = registro.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row]) 
        return data,201
    
    def delete(self, record_id):
        #abort_if_record_doesnt_exist(record_id)
        db = get_db()
        registros = db.execute('delete from refrigeradores where registro = ?',record_id)
        salida = registros.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row])
        return data,201
    
    def put(self,record_id):
        db = get_db()
        args = parser.parse_args()
        fechaStr=timestamp_string()
        valor = args['valor']
        upd=db.execute('update refrigeradores set valor = ?, fecha = ? where registro = ?',[valor,fechaStr,record_id])
        db.commit()
        
        result=upd.fetchall()
        data=[]
        for row in result:
            data.append([x for x in row])
        return data,201

#Class RefrigQuery:Dispone la API GET con selección de parámetro en la URL (?id="No.ID", ?id_refrig="ID Refrigerador", ?var="variable") Tabla refrigeradores
class RefrigQuery(Resource):
    def get(self, record_id=None):
        db = get_db()
        if 'id' in request.args:
            id = request.args['id']
            registro = db.execute('select idnevera, variable, valor, fecha from refrigeradores where registro = ?',id)
        else:
            if 'id_refrig' in request.args:
                id_refrig = request.args['id_refrig']
                registro = db.execute('select * from refrigeradores where idnevera = ?',id_refrig)
            if 'var' in request.args:
                var = request.args['var']
                registro = db.execute('select * from refrigeradores where variable = ?',var)
            if ['id_refrig','var'] in request.args:
                var = request.args['var']
                id_refrig = request.args['id_refrig']
                registro = db.execute('select * from refrigeradores where idnevera = ? AND variable = ?',id_refrig, var)
            if registro == None:
                registro = db.execute('select * from refrigeradores')
        salida = registro.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row]) 
        return data,201
    
    
#Class RefrigList: Dispone las API para leer todos los registros y añadir nuevo registro (PUT=INSERT)
class RefrigList(Resource):
    def get(self):
        db = get_db()
        registros = db.execute('select * from refrigeradores')
        salida = registros.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row])
        return data,201

    def post(self):
        db = get_db()
        args = parser.parse_args()
        valor=args['valor']
        idnevera=args['id_refrig']
        var=args['var']
        fechaStr=timestamp_string()
        req = db.execute('insert into refrigeradores(idnevera,variable,valor,fecha) values (?,?,?,?)',[idnevera,var,valor,fechaStr])
        db.commit()
        result=req.fetchall()
        
        data=[]
        for row in result:
            data.append([x for x in row])
        print(data)
        return data, 201


#######
#Se definen las clases para manejar las APIs de la tabla eiot
#Class Eiot:Protocolos GET, DELETE Y PUT (SELECT, DELETE Y UPDATE) para tabla eiot.
class Eiot(Resource):
    def get(self, record_id):
        #abort_if_record_doesnt_exist(record_id)
        db = get_db()
        registro = db.execute('select iddispositivo, variable, valor, fecha from eiot where registro = ?',record_id)
        salida = registro.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row]) 
        return data,201
    
    def delete(self, record_id):
        #abort_if_record_doesnt_exist(record_id)
        db = get_db()
        registros = db.execute('delete from eiot where registro = ?',record_id)
        salida = registros.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row])
        return data,201
    
    def put(self,record_id):
        db = get_db()
        args = parser.parse_args()
        fechaStr=timestamp_string()
        valor = args['valor']
        upd=db.execute('update eiot set valor = ?, fecha = ? where registro = ?',[valor,fechaStr,record_id])
        db.commit()
        
        result=upd.fetchall()
        data=[]
        for row in result:
            data.append([x for x in row])
        return data,201

#Class EiotQuery:Dispone la API GET con selección de parámetro en la URL (?id="No.ID", ?id_disp="ID Dispositivo", ?var="variable") Tabla eiot
class EiotQuery(Resource):
    def get(self, record_id=None):
        query_parameters = request.args
        db = get_db()
        if 'id' in request.args:
            id = request.args['id']
            registro = db.execute('select iddispositivo, variable, valor, fecha from eiot where registro = ?',id)
        else:
            if 'id_disp' in request.args:
                id_disp = request.args['id_disp']
                registro = db.execute('select * from eiot where iddispositvo = ?',id_disp)
            if 'var' in request.args:
                var = request.args['var']
                registro = db.execute('select * from eiot where variable = ?',var)
            if ['id_disp','var'] in request.args:
                var = request.args['var']
                id_disp = request.args['id_disp']
                registro = db.execute('select * from eiot where iddispositivo = ? AND variable = ?',id_disp, var)
            if registro == None:
                registro = db.execute('select * from eiot')
        salida = registro.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row]) 
        return data,201
    
    
#Class EiotList: Dispone las API para leer todos los registros y añadir nuevo registro (POST=INSERT)
class EiotList(Resource):
    def get(self):
        db = get_db()
        registros = db.execute('select * from eiot')
        salida = registros.fetchall()
        data=[]
        for row in salida:
            data.append([x for x in row])
        return data,201

    def post(self):
        db = get_db()
        args = parser.parse_args()
        valor=args['valor']
        id_disp=args['id_disp']
        var=args['var']
        fechaStr=timestamp_string()
        req = db.execute('insert into eiot(iddispositivo,variable,valor,fecha) values (?,?,?,?)',[id_disp,var,valor,fechaStr])
        db.commit()
        result=req.fetchall()
        
        data=[]
        for row in result:
            data.append([x for x in row])
        print(data)
        return data, 201


##
## Se configuran las API con las clases en las rutas especificadas.
##El parámetro <variable> significa que se reemplaza con una variable en la URL.

api.add_resource(VarList, '/registros')
api.add_resource(Var, '/registros/id_registro/<record_id>')
api.add_resource(VarQuery, '/registros/query')
api.add_resource(RefrigList, '/refrigeradores')
api.add_resource(Refrig, '/refrigeradores/id_registro/<record_id>')
api.add_resource(RefrigQuery, '/refrigeradores/query')
api.add_resource(EiotList, '/eiot')
api.add_resource(Eiot, '/eiot/id_registro/<record_id>')
api.add_resource(EiotQuery, '/eiot/query')


#Funciones para la operación de la base de datos SQLite3.

def connect_db():
    """Connects to the specific database."""
    rv = sqlite3.connect(app.config['DATABASE'])
    rv.row_factory = sqlite3.Row
    return rv

def init_db():
    db = get_db()
    with app.open_resource('schema.sql', mode='r') as f:
        db.cursor().executescript(f.read())
    db.commit()

@app.cli.command('initdb')
def initdb_command():
    """Initializes the database."""
    init_db()
    print('Database initialized.')

def get_db():
    """Opens a new database connection if there is none yet for the
    current application context.
    """
    if not hasattr(g, 'sqlite_db'):
        g.sqlite_db = connect_db()
    return g.sqlite_db
@app.teardown_appcontext
def close_db(error):
    """Closes the database again at the end of the request."""
    if hasattr(g, 'sqlite_db'):
        g.sqlite_db.close()
        #return ""

#### Función para Requests con AJAX #####
@app.route('/suma_numeros')
def suma_numeros():
    a = request.args.get('a', 0, type=int)
    b = request.args.get('b', 0, type=int)
    return jsonify(result=a + b)


###### Actualización En tiempo real de Gráficas #######
@app.route('/actualizacion_tabla_eiot')
def actualizacion_tabla_eiot():
    db = get_db()
    fechaInicio = request.args.get('fechaInicio', 0)
    fechaFinal = request.args.get('fechaFinal', 0)
    tabla = request.args.get('tabla', 0)
    
    tablaEIoT[1]=dynamicGraphs.getTablaEIoT()
    return jsonify(tablaEIoT)

@app.route('/actualizacion_dict_eiot')
def actualizacion_dict_eiot():
    db = get_db()
    dictEiot=dynamicGraphs.getDispVarEiot()
    return jsonify(dictEiot)

@app.route('/actualizacion_graf_eiot')
def actualizacion_graf_eiot():
    db = get_db()
    dictEiot=dynamicGraphs.getDispVarEiot()
    dictGrafEiot=dynamicGraphs.getTablesDVEiot(dictEiot)
    return jsonify(dictGrafEiot)

#
#Se realiza el enrutamiento de cada URL que se considera dentro de la aplicación
#
@app.route('/')
@app.route('/index')
@app.route('/index.html')
def index():
    db = get_db()
    registro = db.execute('select valor, fecha from records limit 10')
    
    salida = registro.fetchall()
    
    tablaEnteraDB=db.execute('select * from records')
    tablaEntera=tablaEnteraDB.fetchall()
    labels=[]
    values=[]
    tablaRegistros=[]
    for row in salida:
        labels.append(row[1])
        values.append(row[0])
    for line in tablaEntera:
        tablaRegistros.append(line)              
    updateTime=labels[-1]
    tablaRegistros=dynamicGraphs.getTablaRegistros()
    tablaRefrigeradores=dynamicGraphs.getTablaRefrigeradores()
    tablaEIoT=dynamicGraphs.getTablaEIoT()
    dictEiot=dynamicGraphs.getDispVarEiot()
    dictRefrig=dynamicGraphs.getDispVarRefrig()
    dictGrafEiot=dynamicGraphs.getTablesDVEiot(dictEiot)
    dictGrafRefrig=dynamicGraphs.getTablesDVRefrig(dictRefrig)
    #print(dictGrafEiot)
##    print("Tabla EIoT")
##    print(tablaEIoT)
##    print("Tabla Registros")
##    print(tablaRegistros)
    titulosTb=['Registros','Refrigeradores','EIoT']
    #labels = ["January", "February", "March", "April", "May", "June", "July", "August"]
    #values = [10, 9, 8, 7, 6, 4, 7, 8]
    return render_template('index2.html', values=values, labels=labels, updateTime=updateTime, tablaRegistros=tablaRegistros,
                           tablaRefrigeradores=tablaRefrigeradores,tablaEIoT=tablaEIoT,titulosTb=titulosTb,dictEiot=dictEiot,dictRefrig=dictRefrig,
                           dictGrafEiot=dictGrafEiot,dictGrafRefrig=dictGrafRefrig)


@app.route('/add', methods=['POST'])
def add_entry():
    if not session.get('logged_in'):
        abort(401)
    db = get_db()
    db.execute('insert into entries (title, text) values (?, ?)',
                 [request.form['title'], request.form['text']])
    db.commit()
    flash('New entry was successfully posted')
    return redirect(url_for('show_entries'))


@app.route('/descargas')
@app.route('/descargas.html')
def descargas():
    db = get_db()
    registro = db.execute('select valor, fecha from records limit 10')
    
    salida = registro.fetchall()
    
    tablaEnteraDB=db.execute('select * from records')
    tablaEntera=tablaEnteraDB.fetchall()
    labels=[]
    values=[]
    tablaRegistros=[]
    for row in salida:
        labels.append(row[1])
        values.append(row[0])
    for line in tablaEntera:
        tablaRegistros.append(line)              
    updateTime=labels[-1]
    tablaRegistros=dynamicGraphs.getTablaRegistros()
    tablaRefrigeradores=dynamicGraphs.getTablaRefrigeradores()
    tablaEIoT=dynamicGraphs.getTablaEIoT()
    dictEiot=dynamicGraphs.getDispVarEiot()
    dictRefrig=dynamicGraphs.getDispVarRefrig()
    dictGrafEiot=dynamicGraphs.getTablesDVEiot(dictEiot)
    dictGrafRefrig=dynamicGraphs.getTablesDVRefrig(dictRefrig)
    #print(dictGrafEiot)
##    print("Tabla EIoT")
##    print(tablaEIoT)
##    print("Tabla Registros")
##    print(tablaRegistros)
    titulosTb=['Registros','Refrigeradores','EIoT']
    return render_template('descargas.html', values=values, labels=labels, updateTime=updateTime, tablaRegistros=tablaRegistros,
                           tablaRefrigeradores=tablaRefrigeradores,tablaEIoT=tablaEIoT,titulosTb=titulosTb,dictEiot=dictEiot,dictRefrig=dictRefrig,
                           dictGrafEiot=dictGrafEiot,dictGrafRefrig=dictGrafRefrig)

@app.route('/descargas/descargararchivoseiot/csv')
def descargarArchivosEiotCSV():
    db = get_db()
    dynamicGraphs.crearArchivosEiot()
    return render_template('descargas.html')

@app.route('/descargas/descargararchivosrefrigeradores/csv')
def descargarArchivosRefrigCSV():
    db = get_db()
    dynamicGraphs.crearArchivosRefrig()
    return render_template('descargas.html')

@app.route('/descargas/descargararchivoseiot/xlsx')
def descargarArchivosEiotxlsx():
    db = get_db()
    dynamicGraphs.crearArchivosEiot()
    return render_template('descargas.html')

@app.route('/descargas/descargararchivosrefrigeradores/xlsx')
def descargarArchivosRefrigxlsx():
    db = get_db()
    dynamicGraphs.crearArchivosRefrig()
    return render_template('descargas.html')

@app.route('/blank')
@app.route('/blank.html')
def blank():
    return render_template('blank.html')

@app.route('/cards')
@app.route('/cards.html')
def cards():
    return render_template('cards.html')

@app.route('/charts')
@app.route('/charts.html')
def charts():
    db = get_db()
    registro = db.execute('select valor, fecha from records limit 10')
    
    salida = registro.fetchall()
    
    tablaEnteraDB=db.execute('select * from records')
    tablaEntera=tablaEnteraDB.fetchall()
    labels=[]
    values=[]
    tablaRegistros=[]
    for row in salida:
        labels.append(row[1])
        values.append(row[0])
    for line in tablaEntera:
        tablaRegistros.append(line)              
    updateTime=labels[-1]
    tablaRegistros=dynamicGraphs.getTablaRegistros()
    tablaRefrigeradores=dynamicGraphs.getTablaRefrigeradores()
    tablaEIoT=dynamicGraphs.getTablaEIoT()
    dictEiot=dynamicGraphs.getDispVarEiot()
    dictRefrig=dynamicGraphs.getDispVarRefrig()
    dictGrafEiot=dynamicGraphs.getTablesDVEiot(dictEiot)
    dictGrafRefrig=dynamicGraphs.getTablesDVRefrig(dictRefrig)
    #print(dictGrafEiot)
##    print("Tabla EIoT")
##    print(tablaEIoT)
##    print("Tabla Registros")
##    print(tablaRegistros)
    titulosTb=['Registros','Refrigeradores','EIoT']
    return render_template('graficas.html', values=values, labels=labels, updateTime=updateTime, tablaRegistros=tablaRegistros,
                           tablaRefrigeradores=tablaRefrigeradores,tablaEIoT=tablaEIoT,titulosTb=titulosTb,dictEiot=dictEiot,dictRefrig=dictRefrig,
                           dictGrafEiot=dictGrafEiot,dictGrafRefrig=dictGrafRefrig)

@app.route('/forgot-password')
@app.route('/forgot-password.html')
def forgotpassword():
    return render_template('forgot-password.html')

'''@app.route('/login')
@app.route('/login.html')
def login():
    return render_template('login.html')'''

@app.route('/login', methods=['GET', 'POST'])
@app.route('/login.html', methods=['GET', 'POST'])
def login():
    error = None
    if request.method == 'POST':
        if request.form['username'] != app.config['USERNAME']:
            error = 'Invalid username'
            print(error)
            flash(error)
            return redirect('index')
        elif request.form['password'] != app.config['PASSWORD']:
            error = 'Invalid password'
            print(error)
            flash(error)
            return redirect('index')
        else:
            session['logged_in'] = True
            flash('You were logged in')
            return redirect('index')
    return render_template('login.html', error=error)

@app.route('/logout')
def logout():
    session.pop('logged_in', None)
    flash('You were logged out')
    return redirect('login')

@app.route('/navbar')
@app.route('/navbar.html')
def navbar():
    return render_template('navbar.html')

@app.route('/register')
@app.route('/register.html')
def register():
    return render_template('register.html')

@app.route('/tables')
@app.route('/tables.html')
def tables():
    db = get_db()
    registro = db.execute('select valor, fecha from records limit 10')
    
    salida = registro.fetchall()
    
    tablaEnteraDB=db.execute('select * from records')
    tablaEntera=tablaEnteraDB.fetchall()
    labels=[]
    values=[]
    tablaRegistros=[]
    for row in salida:
        labels.append(row[1])
        values.append(row[0])
    for line in tablaEntera:
        tablaRegistros.append(line)              
    updateTime=labels[-1]
    tablaRegistros=dynamicGraphs.getTablaRegistros()
    tablaRefrigeradores=dynamicGraphs.getTablaRefrigeradores()
    tablaEIoT=dynamicGraphs.getTablaEIoT()
    dictEiot=dynamicGraphs.getDispVarEiot()
    dictRefrig=dynamicGraphs.getDispVarRefrig()
    dictGrafEiot=dynamicGraphs.getTablesDVEiot(dictEiot)
    dictGrafRefrig=dynamicGraphs.getTablesDVRefrig(dictRefrig)
    #print(dictGrafEiot)
##    print("Tabla EIoT")
##    print(tablaEIoT)
##    print("Tabla Registros")
##    print(tablaRegistros)
    titulosTb=['Registros','Refrigeradores','EIoT']
    return render_template('tablas.html', values=values, labels=labels, updateTime=updateTime, tablaRegistros=tablaRegistros,
                           tablaRefrigeradores=tablaRefrigeradores,tablaEIoT=tablaEIoT,titulosTb=titulosTb,dictEiot=dictEiot,dictRefrig=dictRefrig,
                           dictGrafEiot=dictGrafEiot,dictGrafRefrig=dictGrafRefrig)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
    
    