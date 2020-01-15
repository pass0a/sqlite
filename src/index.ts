const sqlite = require('./sqlite.passoa');
class SqlOp {
	private inst: any;
	constructor(ptr: any) {
		this.inst = ptr;
	}
	exec(...args: any[]) {
		var tmp = Array.prototype.slice.call(args);
		tmp.unshift(this.inst);
		return sqlite.sqlite_exec.apply(this, tmp);
	}
	close() {
		return sqlite.sqlite_close(this.inst);
	}
	value(id: number) {
		return sqlite.sqlite_get(this.inst, id);
	}
	next() {
		return sqlite.sqlite_next(this.inst);
	}
	test() {
		return sqlite.sqlite_test(this.inst);
	}
}
export function open(path: string, fn: (code: number, info: number) => void) {
	return new SqlOp(sqlite.sqlite_open(path, fn));
}
