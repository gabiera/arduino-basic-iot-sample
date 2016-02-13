"""
This script runs the application using a development server.
It contains the definition of routes and views for the application.
"""

from flask import Flask, request
import datetime
app = Flask(__name__)
app.debug = True
# Make the WSGI interface available at the top level so wfastcgi can get it.
wsgi_app = app.wsgi_app
devices = []

@app.route('/')
def hi():
    return 'hello world'


@app.route('/Devices/<devid>', methods=['POST'] )
def create_device(devid):
    curdevice = None
    for device in devices:
        if device['deviceid'] == devid:
            curdevice=device
            break
    if curdevice is not None:
        print(curdevice)
        return '<'+str(1)+'|'+str(curdevice['ledstatus'])+'>'
    else:
        print(devid)
        curdevice = { 'deviceid':devid, 'ledstatus':0 }
        devices.append(curdevice)
        return '<'+str(0)+'>'

@app.route('/Devices/<devid>', methods = [ 'GET' ] )
def get_device(devid): 
    curdevice = None
    for device in devices:
        if device['deviceid'] == devid:
            curdevice=device
            break
    if curdevice is not None:
        return '<'+str(1)+'|'+str(curdevice['ledstatus'])+'>'

@app.route('/Devices/<devid>/state', methods = ['POST'] )
def recordTemp(devid):
    print(request.form)
    temp = request.form['temp']
    led = request.form['ledstatus']
    with open(devid+'.csv', 'a') as filehandle:
        filehandle.write(str(datetime.datetime.utcnow())+','+temp+','+led+'\n')
    curstate = get_device(devid)
    return curstate

@app.route('/Devices/<devid>/led', methods = ['POST'] )
def changeled(devid):
    curdevice = None
    newledstatus = int(request.form['ledstatus'])
    for device in devices:
        if device['deviceid'] == devid:
            curdevice = device
            break
    curdevice['ledstatus'] = newledstatus
    return '<ok>'

if __name__ == '__main__':
    import os
    HOST = '0.0.0.0'
    PORT = 5555
    app.run(HOST, PORT)
