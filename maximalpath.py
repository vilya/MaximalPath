#!/usr/bin/env python
import sys
import time


class Graph(object):
  def __init__(self):
    self.nodes = set([])
    self.edges = {}


  def addEdge(self, fromNode, toNode):
    fromNode = fromNode.lower()
    toNode = toNode.lower()

    self.nodes.add(fromNode)
    self.nodes.add(toNode)

    if fromNode not in self.edges:
      self.edges[fromNode] = set([toNode])
    else:
      self.edges[fromNode].add(toNode)

    if toNode not in self.edges:
      self.edges[toNode] = set([fromNode])
    else:
      self.edges[toNode].add(fromNode)


  def parse(self, f):
    for line in f:
      edge = line.strip()
      fromNode = edge[:3]
      toNode = edge[3:]
      self.addEdge(fromNode, toNode)


  def printDot(self, f):
    print >> f, "graph Sample {"
    for fromNode in self.nodes:
      if fromNode not in self.edges:
        continue
      for toNode in self.edges[fromNode]:
        print >> f, "  %s -- %s;" % (fromNode, toNode)
    print >> f, "}"



def MaximalPath(graph, start, num, path=[], count=0):
  try:
    if path == []:
      start = start.lower()
      return MaximalPath(graph, start, num, [start], count)

    if path[-1] in graph.edges:
      nextNodes = sorted([n for n in graph.edges[path[-1]] if n not in path])
      if nextNodes != []:
        newCount = 0
        try:
          for n in nextNodes:
            newCount += MaximalPath(graph, start, num, path + [n], count + newCount)
          return newCount
        except KeyboardInterrupt:
          print "\n%d paths found so far" % (count + newCount)
          raise Exception("Aborted")

    #if count < num:
    print "".join(path).upper()

    return 1
  except KeyboardInterrupt:
    print "\n%d paths found so far" % count
    raise Exception("Aborted")


    
def main():
  if len(sys.argv) != 3:
    print >> sys.stderr, "Usage: %s <graph> <nodes>" % sys.argv[0]
    sys.exit(-1)

  f = open(sys.argv[1])
  graph = Graph()
  graph.parse(f)
  f.close()

  f = open(sys.argv[2])
  num = int(f.next().strip())
  for line in f:
    start = line.strip()
    print "First %d lexicographic paths from %s:" % (num, start)
    count = MaximalPath(graph, start, num)
    print "Total maximal paths starting from %s: %d" % (start, count)
    print ""

  #graph.printDot(sys.stdout)


if __name__ == '__main__':
  begin = time.clock()
  try:
    main()
  except Exception, e:
    print str(e)
  end = time.clock()
  print >> sys.stderr, "Execution completed in %5.4f seconds" % (end - begin)

