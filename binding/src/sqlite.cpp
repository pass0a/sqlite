#include "plugin.h"
#include "sqlite3.h"
#include <iostream>
#include <memory>
#include <map>

struct pa_plugin gp;


class SqlOp
    :public std::enable_shared_from_this<SqlOp> {
public:
    static SqlOp* create(pa_context* ctx) {
        auto p = std::make_shared<SqlOp>(ctx);
        mgr.insert(std::make_pair(p.get(), p));
        return p.get();
    }
    static bool find(SqlOp* p) {
        auto it=mgr.find(p);
        return it != mgr.end();
    }
public:
    SqlOp(pa_context* ctx)
        :ref_(-1),ctx_(ctx), reloaded_(0),stmt(0){
        bindRef();
    }
    virtual ~SqlOp() {
        if (ref_ != -1) {
            gp.eval_string(ctx_, "(passoa_callbacks.free.bind(passoa_callbacks))");
            gp.push_int(ctx_, ref_);
            gp.call(ctx_, 1);
            gp.pop(ctx_);
        }
    }
public:
    int open(const char* path) {
        int ret=sqlite3_open(path, &p);
        onError(ret, sqlite3_errmsg(p));
        return ret;
    }
    int get(pa_context* ctx) {
        int ret = 0; 
        int idx = 0;
        int size=sqlite3_column_count(stmt);
        

        if (gp.is_number(ctx, 1)) {
            idx = gp.get_int(ctx,1);
            switch (sqlite3_column_type(stmt,idx)) {
            case 1:
                gp.push_int(ctx,sqlite3_column_int(stmt, idx));
                break;
            case 2:
                gp.push_number(ctx,sqlite3_column_double(stmt, idx));
                break;
            case 3:
                gp.push_string(ctx, (const char*)sqlite3_column_text(stmt, idx));
                break;
            case 4:
                {
                    int len=sqlite3_column_bytes(stmt, idx);
                    void* buf=gp.push_fixed_buffer(ctx, len);
                    memcpy(buf, sqlite3_column_blob(stmt, idx), len);
                }
                break;
            case 5:
                gp.push_null(ctx);
                break;
            default:
                gp.push_undefined(ctx);
                break;
            }
        }
        return 1;
    }
    int exec(pa_context* ctx) {
        int ret = 0;
        int idx=0,len = gp.get_top(ctx);
        const char* sql = NULL;
        
        reloaded_ = 0;
        if(stmt)
            sqlite3_finalize(stmt);
        
        if (!gp.is_string(ctx, 1)) {
            goto end;
        }
        sql = gp.get_string(ctx, 1);
        ret=sqlite3_prepare(p, sql, -1, &stmt, NULL);
        onError(ret, sqlite3_errmsg(p));
        
        if (ret) {
            goto end;
        }
        for (auto i = 2; i < len; i++) {
            idx = i-1;
            switch (gp.get_type(ctx, i))
            {
            case 1:
            case 2:
                ret=sqlite3_bind_null(stmt, idx);
                break;
            case 3:
                ret=sqlite3_bind_int(stmt, idx, gp.get_int(ctx, i));
                break;
            case 4:
                ret=sqlite3_bind_double(stmt, idx, gp.get_number(ctx, i));
                break;
            case 5:
                ret=sqlite3_bind_text(stmt, idx, gp.get_string(ctx, i), -1, SQLITE_STATIC);
                break;
            case 6:
                if (gp.is_buffer_data(ctx, i)) {
                    void* buf = 0;
                    int size = 0;
                    buf = gp.get_buffer_data(ctx, i, &size);
                    ret=sqlite3_bind_blob(stmt, idx, buf, size, NULL);
                }
                else {
                    onError(-1, "sqlite not allowed object!!!");
                }
                break;
            default:
                onError(-1, "sqlite only allowed bool,number,string,buffer!!!");
                break;
            }
            onError(ret, sqlite3_errmsg(p));
        }
        ret = sqlite3_step(stmt);
        onError(ret, sqlite3_errmsg(p));

        if (ret == SQLITE_ROW) {
            reloaded_ = 1;
        }
        
    end:
        gp.push_int(ctx,ret);
        return 1;
    }
    int next(pa_context* ctx) {
        if (reloaded_) {
            reloaded_ = 0;
        }else{
            int ret = sqlite3_step(stmt);
            onError(ret, sqlite3_errmsg(p));
            return ret == SQLITE_ROW;
        }
        return 1;
    }
    int close() {
        sqlite3_finalize(stmt);
        return sqlite3_close(p);
    }
private:
    void bindRef() {
        gp.eval_string(ctx_, "(passoa_callbacks.push.bind(passoa_callbacks))");
        gp.dup(ctx_, -2);
        gp.call(ctx_, 1);
        if (gp.is_number(ctx_, -1)) {
            ref_ = gp.get_int(ctx_, -1);
        }
        gp.pop(ctx_);
    }
    void onError(int code,const char* info) {
        switch (code) {
        case SQLITE_OK:
        case SQLITE_ROW:
        case SQLITE_DONE:
            break;
        default:
            gp.eval_string(ctx_, "(passoa_callbacks.call.bind(passoa_callbacks))");
            gp.push_int(ctx_, ref_);
            gp.push_int(ctx_, code);
            gp.push_string(ctx_, info);
            gp.call(ctx_, 3);
            gp.pop(ctx_);
            break;
        }
    }
    void onReaded(char** argv,char** colname, int len) {
        int idx=0;
        gp.eval_string(ctx_, "(passoa_callbacks.call.bind(passoa_callbacks))");
        gp.push_int(ctx_, ref_);
        gp.push_string(ctx_,"data");
        idx=gp.push_array(ctx_);
        for (int i = 0; i < len; i++) {
            gp.push_string(ctx_, argv[i]);
            gp.put_prop_index(ctx_, idx, i);
        }
        idx = gp.push_array(ctx_);
        for (int i = 0; i < len; i++) {
            gp.push_string(ctx_, colname[i]);
            gp.put_prop_index(ctx_, idx, i);
        }
        gp.call(ctx_, 3);
        gp.pop(ctx_);
    }
    static int callback(void *NotUsed, int argc, char **argv, char **colname) {
        SqlOp* p = (SqlOp*)NotUsed;
        p->onReaded(argv,colname,argc);
        return 0;
    }
private:
    int ref_;
    pa_context *ctx_;
    sqlite3 *p;
    sqlite3_stmt *stmt;
    static std::map<SqlOp*, std::shared_ptr<SqlOp>> mgr;
    int reloaded_;
};
std::map<SqlOp*, std::shared_ptr<SqlOp>> SqlOp::mgr;
static int idx=0;
class Print :public FnPtr {
public:
	virtual void run() {
		std::cout << ++idx <<"call Print with io.post!!!" << std::endl;
	}
	
};
int sqlite_open(pa_context* ctx) {
	
    if (gp.is_string(ctx, 0)) {
        SqlOp* p=SqlOp::create(ctx);
        p->open(gp.get_string(ctx,0));
        gp.push_pointer(ctx, p);
    }
    else {
        gp.push_undefined(ctx);
    }
    return 1;
}
int sqlite_exec(pa_context* ctx) {
	int ret = -1;
    if (gp.is_pointer(ctx,0)) {
        SqlOp* p=(SqlOp*)gp.get_pointer(ctx, 0);
        if (SqlOp::find(p)) {
			ret=p->exec(ctx);
        }
    }
	gp.push_int(ctx, -1);
	return 1;
}
int sqlite_next(pa_context* ctx) {
	int ret = -1;
    if (gp.is_pointer(ctx, 0)) {
        SqlOp* p = (SqlOp*)gp.get_pointer(ctx, 0);
        if (SqlOp::find(p)) {
			ret= p->next(ctx);
        }
    }
	gp.push_int(ctx, ret);
	return 1;
}
int sqlite_get(pa_context* ctx) {

    if (gp.is_pointer(ctx, 0)) {
        SqlOp* p = (SqlOp*)gp.get_pointer(ctx, 0);
        if (SqlOp::find(p)) {
			return p->get(ctx);
        }
	}
	gp.push_undefined(ctx);
	return 1;
}
int sqlite_close(pa_context* ctx) {
	int ret = -1;
    if (gp.is_pointer(ctx, 0)){
        SqlOp* p = (SqlOp*)gp.get_pointer(ctx, 0);
        if (SqlOp::find(p)) {
			ret= p->close();
        }
    }
	gp.push_int(ctx, ret);
    return 1;
}
int sqlite_test(pa_context* ctx) {
	gp.post(new Print());
	return 0;
}
static const pa_function_list_entry my_module_funcs[] = {
    { "sqlite_open", sqlite_open, PA_VARARGS /*nargs*/ },
    { "sqlite_exec", sqlite_exec, PA_VARARGS /*nargs*/ },
    { "sqlite_close", sqlite_close, PA_VARARGS /*nargs*/ },
    { "sqlite_next", sqlite_next, PA_VARARGS /*nargs*/ },
    { "sqlite_get", sqlite_get, PA_VARARGS /*nargs*/ },
	{ "sqlite_test", sqlite_test, PA_VARARGS /*nargs*/ },
    { NULL, NULL, 0 }
};
int passoa_init(pa_plugin p) {
    gp = p;
    gp.put_function_list(p.ctx, -1, my_module_funcs);
    return 0;
}
int passoa_exit() {
    return 0;
}
