drop table if exists records;
create table records (
  registro integer primary key autoincrement,
  valor real,
  fecha text
);

drop table if exists refrigeradores;
create table refrigeradores (
  registro integer primary key autoincrement,
  idnevera text,
  variable text,
  valor numeric,
  fecha text
);

drop table if exists eiot;
create table refrigeradores (
  registro integer primary key autoincrement,
  iddispositivo text,
  variable text,
  valor numeric,
  fecha text
);

drop table if exists entries;
create table entries (
  id integer primary key autoincrement,
  title text not null,
  'text' text not null
);