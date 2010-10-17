#!/usr/bin/env python
import sys

def main():
  if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: %s <graph>" % sys.argv[0]
    sys.exit(-1)
  
  graph = {}
  edgeCount = 0
  duplicates = 0
  with open(sys.argv[1]) as f:
    for line in f:
      line = line.upper()
      fromNode = line[:3]
      toNode = line[3:6]

      if fromNode in graph and toNode in graph[fromNode]:
        duplicates += 1
        print line,

      if fromNode not in graph:
        graph[fromNode] = [toNode]
      else:
        graph[fromNode].append(toNode)

      if toNode not in graph:
        graph[toNode] = [fromNode]
      else:
        graph[toNode].append(fromNode)

      edgeCount += 1

  print "%d nodes" % len(graph)
  print "%d edges" % edgeCount
  print "%d to %d edges per node" % (
    min([len(edges) for _, edges in graph.iteritems()]),
    max([len(edges) for _, edges in graph.iteritems()])
  )
  print "%d self edges" % sum([1 for node, edges in graph.iteritems() if node in edges])
  print "%d duplicate edges" % duplicates


if __name__ == '__main__':
  main()

