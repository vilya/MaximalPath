#!/usr/bin/env python
import sys


def main():
  if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: %s <file>" % sys.argv[0]
    sys.exit(-1)

  with open(sys.argv[1]) as f:
    paths = sorted(f.readlines())

  dups = 0
  nonmax = 0
  prevpath = "-"
  for path in paths:
    if path == prevpath:
      dups += 1
      print "DUP:   ", prevpath,
    elif path.startswith(prevpath):
      nonmax += 1
      print "NONMAX:", prevpath,
    prevpath = path
  
  print "%d duplicate paths" % dups
  print "%d non-maximal paths" % nonmax


if __name__ == '__main__':
  main()

