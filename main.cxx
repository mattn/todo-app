#include <iostream>
#include <iomanip>
#include <sstream>
#include <codecvt>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>

#ifdef _WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x05010300
#include <windows.h>
#include <fcntl.h>
#endif

static inline std::wstring
to_wstring(const char* pmcs) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(pmcs);
}

static inline std::string
from_wstring(const wchar_t *pwcs) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(pwcs);
}

static inline std::string
from_locale(const char* pmcs) {
#ifdef _WIN32
    size_t len = MultiByteToWideChar(GetACP(), 0, pmcs, -1, NULL, 0);
    wchar_t* ret = (wchar_t*) malloc((len + 1) * sizeof(wchar_t));
    len = MultiByteToWideChar(GetACP(), 0, pmcs, -1, ret, len);
    ret[len] = 0;
    std::string result = from_wstring(ret);
    free(ret);
    return result;
#else
    return std::string(pmcs);
#endif
}

static void
todo_add(std::string dsn, std::string text) {
  soci::session sql(soci::postgresql, dsn);
  sql << "insert into todo(text) values(:rtext)", soci::use(text);
}

static void
todo_list(std::string dsn) {
  soci::session sql(soci::postgresql, dsn);
  int id;
  std::string text;
  int done;
  std::tm created_at;
  soci::rowset<soci::row> rs = (sql.prepare << "select id, text, done, created_at from todo order by id");
  soci::rowset<soci::row>::const_iterator it;
  for (it = rs.begin(); it != rs.end(); it++) {
    soci::row const &r1 = (*it);
    id = r1.get<int>(0);
    text = r1.get<std::string>(1);
    done = r1.get<int>(2);
    created_at = r1.get<std::tm>(3);
    std::stringstream ss;
    ss << std::put_time(&created_at, "%Y/%m/%d %H-%M-%S");
    std::wcout << id
        << " " << to_wstring(ss.str().c_str())
        << " " << "[" << (done ? "*" : " ") << "]"
        << " " << to_wstring(text.c_str())
        << std::endl;
  }
}

static void
todo_done(std::string dsn, int id) {
  soci::session sql(soci::postgresql, dsn);
  sql << "update todo set done = true where id = :rid", soci::use(id);
}

static void
todo_undone(std::string dsn, int id) {
  soci::session sql(soci::postgresql, dsn);
  sql << "update todo set done = false where id = :rid", soci::use(id);
}

static void
todo_delete(std::string dsn, int id) {
  soci::session sql(soci::postgresql, dsn);
  sql << "delete from todo where id = :rid", soci::use(id);
}

static void
todo_init(std::string dsn) {
  soci::session sql(soci::postgresql, dsn);
  try { sql << "drop table todo"; }
  catch (soci::soci_error const &) {}
  sql <<
    "create table todo ("
    "    id serial primary key"
    "    , text varchar(500)"
    "    , done bool default false"
    "    , created_at timestamp default current_timestamp"
    ")";
}

int
main(int argc, char *argv[]) {
#ifdef _WIN32
  setmode(fileno(stdout), _O_U16TEXT);
#endif
  std::string dsn = std::getenv("TODO_DSN");
  std::string command = argc >=  2 ? argv[1] : "list";
  if (command == "init") {
    todo_init(dsn);
  } else if (command == "list") {
    todo_list(dsn);
  } else if (command == "add" && argc == 3) {
    todo_add(dsn, from_locale(argv[2]));
  } else if (command == "done" && argc == 3) {
    auto i = std::stoi(argv[2]);
    todo_done(dsn, i);
  } else if (command == "undone" && argc == 3) {
    auto i = std::stoi(argv[2]);
    todo_undone(dsn, i);
  } else if (command == "delete" && argc == 3) {
    auto i = std::stoi(argv[2]);
    todo_delete(dsn, i);
  } else {
    std::wcout
        << "todo [sub-command]" << std::endl
        << "  list       : list todo items" << std::endl
        << "  add [text] : add todo item" << std::endl
        << "  done [id]  : done the todo" << std::endl
        << "  undone [id]: undone the todo" << std::endl
        << "  delete [id]: delete the todo" << std::endl;
  }
  return 0;
}
