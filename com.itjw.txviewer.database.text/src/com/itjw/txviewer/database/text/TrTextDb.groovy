package com.itjw.txviewer.database.text;

import java.io.InputStream;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import com.itjw.txviewer.database.ITrDb;
import com.itjw.txviewer.database.ITrGenerator;
import com.itjw.txviewer.database.ITrHierNode;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.InputFormatException;
import com.itjw.txviewer.database.EventTime

public class TrTextDb extends TrHierNode implements ITrDb{

	private EventTime maxTime;
	def streams = []
	def childs = []
	
	public String getFullName() {
		return getName();
	}

	@Override
	public EventTime getMaxTime() {
		return maxTime;
	}

	public List<ITrStream> getAllStreams() {
		return new LinkedList<ITrStream>(streams);
	}

	public List<ITrHierNode>  getChildNodes() {
		return childs.sort{it.name};
	}

	public Map<Long, ITrGenerator> getGeneratorsById() {
		TreeMap<Long, ITrGenerator> res = new TreeMap<Long, ITrGenerator>();
		streams.each{TrStream stream -> stream.generators.each{res.put(it.id, id)} }
		return res;
	}

	void clear(){
		streams = []
		maxTime=new EventTime(0, "ns")
	}
	
	void load(InputStream inp) throws InputFormatException {
		parseInput(inp)
	}

	void parseFromTextFile(filename){
		def file = new File(filename);
		this.databaseName = filname;
		parseInput(file)
	}
	
	private def parseInput(input){
		def streamsById = [:]
		def generatorsById = [:]
		def transactionsById = [:]
		TrGenerator generator
		Transaction transaction
		boolean endTransaction=false
		def matcher
		input.eachLine { line ->
			def tokens = line.split(/\s+/)
			switch(tokens[0]){
				case "scv_tr_stream":
				case "scv_tr_generator":
				case "begin_attribute":
				case "end_attribute":
					if ((matcher = line =~ /^scv_tr_stream\s+\(ID (\d+),\s+name\s+"([^"]+)",\s+kind\s+"([^"]+)"\)$/)) {
						def id = Integer.parseInt(matcher[0][1])
						def stream = new TrStream(id, this, matcher[0][2], matcher[0][3])
						streams<<stream
						streamsById[id]=stream
					} else if ((matcher = line =~ /^scv_tr_generator\s+\(ID\s+(\d+),\s+name\s+"([^"]+)",\s+scv_tr_stream\s+(\d+),$/)) {
						def id = Integer.parseInt(matcher[0][1])
						TrStream stream=streamsById[Integer.parseInt(matcher[0][3])]
						generator=new TrGenerator(id, stream, matcher[0][2])
						stream.generators<<generator
						generatorsById[id]=generator
					} else if ((matcher = line =~ /^begin_attribute \(ID (\d+), name "([^"]+)", type "([^"]+)"\)$/)) {
						generator.begin_attrs << TrAttrType.getAttrType(matcher[0][2], matcher[0][3])
					} else if ((matcher = line =~ /^end_attribute \(ID (\d+), name "([^"]+)", type "([^"]+)"\)$/)) {
						generator.end_attrs << TrAttrType.getAttrType(matcher[0][2], matcher[0][3])
					}
					break;
				case ")":
					generator=null
					break
				case "tx_begin"://matcher = line =~ /^tx_begin\s+(\d+)\s+(\d+)\s+(\d+)\s+([munpf]?s)/
					def id = Integer.parseInt(tokens[1])
					TrGenerator gen=generatorsById[Integer.parseInt(tokens[2])]
					transaction = new Transaction(id, gen,new EventTime(Integer.parseInt(tokens[3]), tokens[4]))
					gen.transactions << transaction
					transactionsById[id]= transaction
					gen.begin_attrs_idx=0;
					maxTime = maxTime>transaction.beginTime?maxTime:transaction.beginTime
					endTransaction=false
					break
				case "tx_end"://matcher = line =~ /^tx_end\s+(\d+)\s+(\d+)\s+(\d+)\s+([munpf]?s)/
					def id = Integer.parseInt(tokens[1])
					transaction = transactionsById[id]
					assert Integer.parseInt(tokens[2])==transaction.generator.id
					transaction.endTime = new EventTime(Integer.parseInt(tokens[3]), tokens[4])
					transaction.generator.end_attrs_idx=0;
					maxTime = maxTime>transaction.endTime?maxTime:transaction.endTime
					endTransaction=true
					break
				case "tx_record_attribute"://matcher = line =~ /^tx_record_attribute\s+(\d+)\s+"([^"]+)"\s+(\S+)\s*=\s*(.+)$/
					def id = Integer.parseInt(tokens[1])
					transactionsById[id].attributes.add(new TrAttribute(tokens[2][1..-2], tokens[3], tokens[5..-1].join(' ')))
					break
				case "a"://matcher = line =~ /^a\s+(.+)$/
					if(endTransaction){
						transaction.end_attrs << new TrAttribute(transaction.generator.end_attrs[0], tokens[1])
					} else {
						transaction.begin_attrs << new TrAttribute(transaction.generator.begin_attrs[0], tokens[1])
					}
					break
				case "tx_relation"://matcher = line =~ /^tx_relation\s+\"(\S+)\"\s+(\d+)\s+(\d+)$/
					Transaction tr1= transactionsById[Integer.parseInt(tokens[2])]
					Transaction tr2= transactionsById[Integer.parseInt(tokens[3])]
					switch(tokens[1][1..-2]){
						case "CHILD":
							tr1.child<<tr2
							tr2.parent<<tr1
							break
						case "PARENT":
							tr2.child<<tr1
							tr1.parent<<tr2
							break
						case "PREDECESSOR":
							tr2.succ<<tr1
							tr1.pred<<tr2
							break
						case "SUCCESSOR":
							tr1.succ<<tr2
							tr2.pred<<tr1
							break
						default:
							println "Relationship '${tokens[1]}' not yet implemented"
					}
					break
				default:
					println "Don't know what to do with: '$line'"

			}
		}
		linkTransactions()
		addHierarchyNodes()
	}

	def linkTransactions(){
		streams.generators?.flatten().each {TrGenerator gen ->
			def sortedTx = gen.transactions.sort{it.beginTime}
			if(sortedTx.size()>1)
				for(int i=1;i<sortedTx.size(); i++){
					sortedTx[i].prev= sortedTx[i-1]
					sortedTx[i-1].next = sortedTx[i]
				}
		}
	}
	
	def addHierarchyNodes(){
		streams.each{ TrStream stream->
			def hier = stream.fullName.split(/\./)
			ITrHierNode node = this
			hier.each { name ->
				def n1 = node.childNodes.find{it.name == name}
				if(name == hier[-1]){ //leaf
					if(n1!=null) {
						if(n1 instanceof TrHierNode){
							node.childNodes.remove(n1)
							stream.childNodes.addAll(n1.childNodes)
						} else {
							throw new InputFormatException()
						}
					}
					stream.name=name
					node.childNodes<<stream
					node=stream
				} else { // intermediate
					if(n1 != null) {
						node=n1
					} else {
						TrHierNode newNode = new TrHierNode(name)
						node.childNodes<<newNode
						node=newNode
					}
				}
			}
		}
	}

}
