import sqlite3
import json


def createTables():
    db_conn = sqlite3.connect("db.db")
    sql = """
        CREATE TABLE resident (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            room_nr INTEGER NOT NULL,
            awake_time Text default "07:00:00",
            bed_time Text default "10:00:00"
        )
        """
    cursor = db_conn.cursor()
    cursor.execute(sql)
    cursor.close()

    sql = """
                CREATE TABLE sensor (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                resident_id INTEGER NOT NULL,
                created_at DATE default (datetime('now', 'localtime')) NOT NULL,
                in_bed bool NOT NULL,
                duration int not null, 
                FOREIGN KEY (resident_id) REFERENCES resident (id)
                )
                """
    cursor = db_conn.cursor()
    cursor.execute(sql)
    cursor.close()

    sql = """
                    CREATE TABLE alert (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    resident_id INTEGER NOT NULL,
                    created_at DATE default (datetime('now', 'localtime')) NOT NULL,
                    resolved_at DATE,
                    outcome bool,
                    FOREIGN KEY (resident_id) REFERENCES resident(id)
                    )
                    """
    cursor = db_conn.cursor()
    cursor.execute(sql)
    cursor.close()
    db_conn.close()


def setupPeople(residents):
    db_conn = sqlite3.connect("db.db")
    sql = """
    INSERT INTO resident(name, room_nr,awake_time,bed_time) 
    values (?,?,?,?)
    """
    cursor = db_conn.cursor()
    cursor.executemany(sql, residents)
    db_conn.commit()
    db_conn.close()

"""
residents = [("Jimmy", 202, "07:00:00", "10:00:00"), ("Clara", 105, "07:00:00", "10:05:00"),
             ("John", 109, "13:20:00", "09:15:00")]
setupPeople(residents)
"""

def checkAwake(now, future):
    with sqlite3.connect("db.db") as db_conn:
        sql = """
            Select id 
            FROM resident
            where awake_time >= ? and awake_time < ? 
            """
        cursor = db_conn.cursor()
        arr = (now, future)
        cursor.execute(sql, arr)
        res = cursor.fetchall()
        list = []
        for x in res:
            list += x
        return list

def checkBedtime(now, future):
    with sqlite3.connect("db.db") as db_conn:
        sql = """
            Select id 
            FROM resident
            where bed_time >= ? and bed_time < ? 
            """
        cursor = db_conn.cursor()
        arr = (now, future)
        cursor.execute(sql, arr)
        res = cursor.fetchall()
        list = []
        for x in res:
            list += x
        return list

def logSensorData(topic, payload):
    parsed_data = parsePayload(topic, payload)
    with sqlite3.connect("db.db") as db_conn:
        sql = """
            INSERT into sensor (resident_id, in_bed, duration)
            VALUES (?,?,?)
            """
        cursor = db_conn.cursor()
        cursor.execute(sql, parsed_data)
        db_conn.commit()


def logAlertData(id):
    with sqlite3.connect("db.db") as db_conn:
        sql = """
            INSERT into alert (resident_id)
            VALUES (?)
            """
        cursor = db_conn.cursor()
        cursor.execute(sql, id)
        db_conn.commit()

def getResidentInfo(id):
    with sqlite3.connect("db.db") as db_conn:
        sql = """
                Select id, name, room_nr
                FROM resident
                where id = ? 
                """
        cursor = db_conn.cursor()
        cursor.execute(sql, id)
        result = cursor.fetchone()
        return str(result)

def parsePayload(topic, payload):
    resident_id = topic.split("/")[1]
    splitPayload = payload.split(",")
    in_bed = splitPayload[1]
    duration = splitPayload[0]
    return resident_id, in_bed, duration