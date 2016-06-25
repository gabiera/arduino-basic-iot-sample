"""
This script runs the application using a development server.
It contains the definition of routes and views for the application.
"""

from flask import Flask, request
import datetime, json
app = Flask(__name__)
app.debug = True
# Make the WSGI interface available at the top level so wfastcgi can get it.
wsgi_app = app.wsgi_app
devices = []

from azure.servicebus import ServiceBusService
srvns = 'iotsample0917ns'
key_name = 'rootSAS'
key_value = 'gTTf0bVKzFz2Ufjts8unUQRFMEsQORHwmTzAHtKwJng='  
sbs = ServiceBusService(service_namespace=srvns, shared_access_key_name=key_name, shared_access_key_value=key_value) 
flag = sbs.create_event_hub('iotsample0917')
print(flag)


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
        curdevice = { 'deviceid':devid }
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
        return '<'+str(1)+'>'

@app.route('/Devices/<devid>/state', methods = ['POST'] )
def recordTemp(devid):
    print(request.form)
    temp = request.form['temp']
    tdata = { 'DevId':devid,"Time":str(datetime.datetime.utcnow()),"Temperature":temp }
    sbs.send_event('iotsample0917',json.dumps(tdata))
    curstate = get_device(devid)
    return curstate

if __name__ == '__main__':
    import os
    HOST = os.environ.get('SERVER_HOST', 'localhost')
    try:
        PORT = int(os.environ.get('SERVER_PORT', '5555'))
    except ValueError:
        PORT = 5555
    app.run(HOST, PORT)



