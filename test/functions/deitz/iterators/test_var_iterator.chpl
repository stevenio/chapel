var x: 5*int;

iterator xs var {
  for param i in 1..5 do
    yield x(i);
}

writeln(x);

for e in xs do
  e = 2;

writeln(x);
