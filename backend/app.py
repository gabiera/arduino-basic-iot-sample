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
led = 0

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
        return '<'+str(1)+'>'
    else:
        print(devid)
        curdevice = { 'deviceid':devid, 'ledstate': 0 }
        devices.append(curdevice)
        return '<'+str(0)+'>'

@app.route('/Devices/<devid>', methods = [ 'GET' ] )
def get_device(devid): 
    curdevice = None
    for device in devices:
        if device['deviceid'] == devid:
            curdevice=device
            break
    print(curdevice)
    if curdevice is not None:
        print('hello')
        return '<'+str(curdevice['ledstate'])+'>'

@app.route('/Devices/<devid>/state', methods = ['POST'] )
def recordTemp(devid):
    print(request.form)
    temp = request.form['temp']
    with open(devid+'.csv', 'a') as filehandle:
        filehandle.write(str(datetime.datetime.utcnow())+','+temp+'\n')
    curstate = get_device(devid)
    return curstate

@app.route('/switch/<state>', methods = ['GET'] )
def switchState(state):
    global led
    if state == 'on':
        led = 1
    elif state == 'off':
        led = 0
    return 'led is now ' + str(led)
        

@app.route('/switch/<devid>/<state>', methods = ['GET'] )
def switchdevicestate(devid, state):
    if devid != 'all':
        for device in devices:
            if device['deviceid'] == devid:
                if state == 'on':
                    device['ledstate'] = 1
                elif state == 'off':
                    device['ledstate'] = 0
            break
    else:
        for device in devices:
            if state == 'on':
                device['ledstate'] = 1
            elif state == 'off':
                device['ledstate'] = 0
        
    return 'led is now ' + state

if __name__ == '__main__':
    import os
    HOST = '0.0.0.0'
    PORT = 5555
    app.run(HOST, PORT)