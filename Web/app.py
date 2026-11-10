import requests
import time
from flask import Flask, render_template, request, redirect, url_for, session, Response, jsonify
import mysql.connector
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('Agg')
import io
import base64
import urllib.request

app = Flask(__name__)
app.secret_key = 'vut_project_key'

def get_db():
    return mysql.connector.connect(host='localhost', user='root', password='Rick5198#', database='elderly_care')


ESP32_CAM_URL = "http://172.20.10.10:81/stream" 

# Create a global dictionary to remember the last known state of the room
last_known_state = {
    'temp': 0.0,
    'hum': 0.0,
    'motion': 0,
    'dist': 0.0,
    'light': 0
}

def generate_esp32_frames():
    """Streams data straight from the ESP32-CAM with relaxed timeouts."""
    while True:
        try:
            # Added persistent connection headers and increased timeout to 15 seconds
            headers = {'Connection': 'keep-alive'}
            with requests.get(ESP32_CAM_URL, stream=True, timeout=15, headers=headers) as r:
                r.raise_for_status() 
                for chunk in r.iter_content(chunk_size=2048): # Increased chunk size for faster frame collection
                    if chunk:
                        yield chunk
        except Exception as e:
            print(f"[Camera Error Hub]: {e}")
            time.sleep(1) # Quick breather before trying again
            continue

@app.route('/video_feed')
def video_feed():
    return Response(generate_esp32_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

def generate_graph(df, column, title, color):
    plt.figure(figsize=(8, 4), facecolor='#1e2328') 
    ax = plt.axes()
    ax.set_facecolor('#1e2328')
    plt.plot(df['timestamp'], df[column], marker='o', color=color, linewidth=2)
    plt.title(title, color='white')
    plt.xticks(rotation=45, color='white')
    plt.yticks(color='white')
    plt.grid(color='white', alpha=0.1)
    plt.tight_layout()
    img = io.BytesIO()
    plt.savefig(img, format='png')
    img.seek(0)
    plot_url = base64.b64encode(img.getvalue()).decode('utf8')
    plt.close()
    return plot_url

@app.route('/update', methods=['POST'])
def update_sensor():
    global last_known_state
    try:
        # 1. Update ONLY the sensors that sent data in this specific request
        if 'temp' in request.form: last_known_state['temp'] = request.form.get('temp')
        if 'hum' in request.form: last_known_state['hum'] = request.form.get('hum')
        if 'motion' in request.form: last_known_state['motion'] = request.form.get('motion')
        if 'dist' in request.form: last_known_state['dist'] = request.form.get('dist')
        if 'light' in request.form: last_known_state['light'] = request.form.get('light')

        print(f"Current Room State: {last_known_state}")

        # 2. Save the complete state to the database
        conn = get_db()
        cursor = conn.cursor()
        query = """INSERT INTO sensor_data (temp, hum, motion, dist, light) 
                   VALUES (%s, %s, %s, %s, %s)"""
        
        cursor.execute(query, (
            last_known_state['temp'], 
            last_known_state['hum'], 
            last_known_state['motion'], 
            last_known_state['dist'], 
            last_known_state['light']
        ))
        
        conn.commit()
        cursor.close()
        conn.close()
        
        return "Data Saved Successfully", 200
    except Exception as e:
        print(f"Database Error: {e}")
        return "Error Saving Data", 500

# LIVE API DATA ENDPOINT ---
@app.route('/api/latest-data')
def get_latest_data():
    if 'user' not in session: 
        return jsonify({'error': 'Unauthorized'}), 401
        
    conn = get_db()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT temp, hum, motion, dist, light FROM sensor_data ORDER BY timestamp DESC LIMIT 1")
    latest = cursor.fetchone()
    cursor.close()
    conn.close()
    
    if latest:
        return jsonify(latest)
    return jsonify({'temp': 0, 'hum': 0, 'motion': 0, 'dist': 0, 'light': 0})

# --- AUTH ROUTES ---
@app.route('/')
def index():
    if 'user' in session: return redirect(url_for('dashboard'))
    return render_template('login.html')

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        email = request.form['email']
        password = request.form['password']
        db = get_db()
        cursor = db.cursor()
        cursor.execute("SELECT * FROM users WHERE email=%s AND password=%s", (email, password))
        user = cursor.fetchone()
        cursor.close()
        db.close()
        
        if user:
            session['user'] = email
            return redirect(url_for('dashboard'))
        else:
            return "Invalid Credentials!"
    return render_template('login.html')

@app.route('/signup', methods=['GET', 'POST'])
def signup():
    if request.method == 'POST':
        name = request.form['name']
        surname = request.form['surname']
        email = request.form['email']
        password = request.form['password']
        
        db = get_db()
        cursor = db.cursor()
        try:
            cursor.execute("INSERT INTO users (name, surname, email, password) VALUES (%s, %s, %s, %s)", (name, surname, email, password))
            db.commit()
            return redirect(url_for('login'))
        except:
            return "Username already exists!"
        finally:
            cursor.close()
            db.close()
    return render_template('signup.html')

# --- DASHBOARD & GRAPHS ---
@app.route('/dashboard')
def dashboard():
    if 'user' not in session: return redirect(url_for('login'))
    
    conn = get_db()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 1")
    latest = cursor.fetchone()
    cursor.close()
    conn.close()

    if not latest:
        latest = {'temp': 0, 'hum': 0, 'motion': 0, 'dist': 0, 'light': 0}
        
    return render_template('dashboard.html', latest=latest)

@app.route('/temperature')
def temperature():
    if 'user' not in session: return redirect('login')
    conn = get_db()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT timestamp, temp FROM sensor_data ORDER BY timestamp DESC LIMIT 20")
    df = pd.DataFrame(cursor.fetchall())
    cursor.close()
    conn.close()
    plot_url = ""
    if not df.empty:
        df = df.sort_values('timestamp')
        plot_url = generate_graph(df, 'temp', 'Temperature History (°C)', '#f0a04b')
    return render_template('temperature.html', plot_url=plot_url)

@app.route('/humidity')
def humidity():
    if 'user' not in session: return redirect('login')
    conn = get_db()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT timestamp, hum FROM sensor_data ORDER BY timestamp DESC LIMIT 20")
    df = pd.DataFrame(cursor.fetchall())
    cursor.close()
    conn.close()

    plot_url = ""
    if not df.empty:
        df = df.sort_values('timestamp')
        plot_url = generate_graph(df, 'hum', 'Humidity History (%)', '#7fb3d5')

    return render_template('humidity.html', plot_url=plot_url)

@app.route('/camera')
def camera():
    if 'user' not in session: return redirect('login')
    return render_template('camera.html')

@app.route('/motion')
def motion():
    conn = get_db()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT timestamp, motion FROM sensor_data ORDER BY timestamp DESC LIMIT 50")
    rows = cursor.fetchall()
    cursor.close()
    conn.close()
    return render_template('motion.html', data=rows)

@app.route('/light')
def light():
    if 'user' not in session: 
        return redirect('/login')
        
    conn = get_db()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT timestamp, light FROM sensor_data ORDER BY timestamp DESC LIMIT 20")
    df = pd.DataFrame(cursor.fetchall())
    cursor.close()
    conn.close()

    plot_url = ""
    if not df.empty:
        df = df.sort_values('timestamp')
        plot_url = generate_graph(df, 'light', 'Light Intensity History (Lux)', '#7fb3d5')
        
    return render_template('light.html', plot_url=plot_url)

@app.route('/distance')
def distance():
    if 'user' not in session: 
        return redirect('/login')
        
    conn = get_db()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("SELECT timestamp, dist FROM sensor_data ORDER BY timestamp DESC LIMIT 20")
    df = pd.DataFrame(cursor.fetchall())
    cursor.close()
    conn.close()

    plot_url = ""
    if not df.empty:
        df = df.sort_values('timestamp')
        plot_url = generate_graph(df, 'dist', 'Proximity Distance History (cm)', '#a3e4c4')
        
    return render_template('distance.html', plot_url=plot_url)

@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('login'))

if __name__ == '__main__':
    # FIXED: Added threaded=True so asynchronous UI fetching does not block camera feeds
    app.run(host='0.0.0.0', port=5000, debug=True, threaded=True)