var s : integer = 10;
--writeln("Enter table size");
--read(s);
var table : _data(integer) = _data(integer, s);

for i in 1..s {
  table(i-1) = i;
  writeln(table(i-1));
}
