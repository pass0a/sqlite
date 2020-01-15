const fs = require('fs');
import * as sq from '../src/index';
const p = sq.open('./test.db', (code, info) => {
	console.log(code, info);
});
// var sql = "CREATE TABLE COMPANY(ID INT PRIMARY KEY NOT NULL,NAME TEXT NOT NULL)";
// sq.exec(p,sql);
// sql = "INSERT INTO COMPANY (ID,NAME) VALUES (1, 'Paul'); "
// sq.exec(p,sql);
// sql = "SELECT * from COMPANY";
// sq.exec(p,sql);
let sql = 'CREATE TABLE COMPANY(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,NAME TEXT NOT NULL,PIC BLOB,VAL TEXT)';
p.exec(sql);
sql = "INSERT INTO COMPANY (ID,NAME,PIC,VAL) VALUES (NULL, 'Paul',?,?);";
var buf = Buffer.from([ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ]);
p.exec(sql, buf, '345');
sql = 'select * from COMPANY';
p.exec(sql);
while (p.next()) {
	console.log(p.value(0), p.value(1), p.value(2), p.value(3));
	fs.writeFileSync('test' + p.value(0) + '.png', p.value(2));
}
