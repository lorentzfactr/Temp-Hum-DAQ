#WARNING: THIS CODE IS MESSY. IT LIKELY ISNT PORTABLE. IT LIKELY WONT WORK FOR YOU.
#I am only posting this for some friends on my Discord server that wanted to take a look.
#Proceed with caution. To Do List: everything.

#A lot of these imports are obviously not needed, they are from a tutorial I followed
#that showed me how to make a simple blog website. From that tutorial, I setup a simple
#database to store the temp/hum data, a majority of the stuff could be deleted
#but has not yet. Like I said previously, this needs massive cleanup and editing. 
import os
from datetime import datetime
from flask import Flask, render_template, request, redirect, url_for, app, flash, request
from flask_login import LoginManager, UserMixin, login_user, current_user, logout_user, login_required
from flask_wtf import FlaskForm
from flask_mail import Mail, Message
from wtforms import StringField, PasswordField, SubmitField, BooleanField, ValidationError
from wtforms.validators import DataRequired, Length, Email, EqualTo
from flask_bcrypt import Bcrypt
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime as dt
from datetime import date
from itsdangerous import TimedJSONWebSignatureSerializer, Serializer
import csv

app = Flask(__name__)

#You should use an environment variable and create your own key to protect your DB.
#I created mine using a simple python module that spits out a random hex value.
#It is not configured with my env variables yet. But if you're wondering how to:
#create the variables on your local machine and then use the os.environ.get method to call on them.
#see lines 37/38 as a reference.

app.config['SECRET_KEY'] = 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///site.db'
db = SQLAlchemy(app)

#Haven't integrated mail functions yet...
#app.config['MAIL_SERVER'] = 'smtp.googlemail.com'
#app.config['MAIL_PORT'] = 587
#app.config['MAIL_USE_TLS'] = True
#app.config['MAIL_USERNAME'] = os.environ.get('EMAIL_USER')
#app.config['MAIL_PASSWORD'] = os.environ.get('EMAIL_PASS')
#mail = Mail(app)

class Post(db.Model):
    __tablename__='Fridge_Data'
    id = db.Column(db.Integer, primary_key=True)
    date_posted = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)
    temp = db.Column(db.String(10), nullable=False)
    hum = db.Column(db.String(10), nullable=False)
    motion = db.Column(db.String(10), nullable=False)
    d = db.Column(db.String(20), nullable=False)
    t = db.Column(db.String(20), nullable=False)

    def __repr__(self):
        return f"Post('{self.date_posted}','{self.temp}', '{self.hum}', '{self.motion}')"

def whatDate():
    date_stamp = date.today()
    d = date_stamp.strftime("%m/%d/%y")
    return d

def whatTime():
    time_stamp = dt.now()
    t = time_stamp.strftime("%H:%M:%S")
    return t

def CSVwrite(data):
    file = open("C:/Users/cyrus/Desktop/Projects/Code/webapp/data.csv","a")
    file.write(data)
    file.close()

    
db.create_all()

@app.route("/")
@app.route("/home", methods=['GET','POST'])
def home():

    #get query params from arduino and turn them into strings
    temp1 = str(request.args.get('temp'))
    hum1 = str(request.args.get('hum'))
    motion1 = str(request.args.get('motion'))
   
    
    #get the current date and time
    d1 = whatDate()
    t1 = whatTime()

    data_input = Post(temp=temp1, hum=hum1, motion=motion1, d=d1,t=t1)
    db.session.add(data_input)
    db.session.commit()

    #build readable string for debug console and string for csv data
    dataRead = d1 +" @ " + t1 + " | Temp: " + temp1 + "F, Hum: "+ hum1 + "%, Motion? " + motion1 + "\n"
    dataCSV = d1 + "," + t1 + "," + temp1 + "," + hum1 + "," + motion1 + "\n"
   
    #update the CSV file
    CSVwrite(dataCSV)

    #print debug messages    
    print(" ")
    print(dataRead)
    print(" ")
    return render_template("home.html", temp=temp1, hum=hum1)

@app.route("/data")
def data():
    #page = request.args.get('page', 1, type=int)
    posts = Post.query.order_by(Post.date_posted.desc())
    return render_template('data.html', posts=posts)

@app.errorhandler(404)
def error_404(error):
    return render_template('errors/404.html'), 404

@app.errorhandler(403)
def error_404(error):
    return render_template('errors/403.html'), 403
    
@app.errorhandler(500)
def error_500(error):
    return render_template('errors/500.html'), 500    

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0")