from flask import Flask, render_template, request, redirect, url_for
from datetime import datetime as dt
from datetime import date
import csv


app = Flask(__name__)

def whatDate():
    date_stamp = date.today()
    d = date_stamp.strftime("%m/%d/%y")
    return d

def whatTime():
    time_stamp = dt.now()
    t = time_stamp.strftime("%H:%M:%S")
    return t

def CSVwrite(data):
    file = open("C:/YOURFILEPATH/data.csv","a")         #update the filepath to wherever the csv is stored on your server
    file.write(data)
    file.close()

    

@app.route('/')
def index():

    #get query params from arduino and turn them into strings
    temp = str(request.args.get('temp'))
    hum = str(request.args.get('hum'))
    motion = str(request.args.get('motion'))
    
    #get the current date and time
    d = whatDate()
    t = whatTime()

    #build readable string for debug console and string for csv data
    dataRead = d +" @ " + t + " | Temp: " + temp + "F, Hum: "+ hum + "%, Motion? " + motion + "\n"
    dataCSV = d + "," + t + "," + temp + "," + hum + "," + motion + "\n"
   
    #update the CSV file
    CSVwrite(dataCSV)

    #print debug messages    
    print(" ")
    print(dataRead)
    print(" ")
    return render_template('index.html', temp=temp, hum=hum)


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
