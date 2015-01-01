/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
package com.itjw.txviewer.database.text;

import java.io.InputStream;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import com.itjw.txviewer.database.ITrAttrType;
import com.itjw.txviewer.database.ITrAttribute;
import com.itjw.txviewer.database.ITrDb;
import com.itjw.txviewer.database.ITrGenerator;
import com.itjw.txviewer.database.ITrHierNode;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.InputFormatException;
import com.itjw.txviewer.database.EventTime
import com.itjw.txviewer.database.RelationType

public class TrTextDb extends TrHierNode implements ITrDb{

	private EventTime maxTime;
	
	def streams = []
	
	def relationTypes=[:]
	
	public String getFullName() {
		return getName();
	}

	@Override
	public EventTime getMaxTime() {
		return maxTime;
	}

	public ITrStream getStreamByName(String name){
		streams.find{ITrStream stream-> stream.fullName == name }
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
						ITrStream stream=streamsById[Integer.parseInt(matcher[0][3])]
						generator=new TrGenerator(id, stream, matcher[0][2])
						stream.generators<<generator
						generatorsById[id]=generator
					} else if ((matcher = line =~ /^begin_attribute \(ID (\d+), name "([^"]+)", type "([^"]+)"\)$/)) {
						generator.begin_attrs << TrAttrType.getAttrType(matcher[0][2], matcher[0][3], ITrAttrType.AttributeType.BEGIN)
					} else if ((matcher = line =~ /^end_attribute \(ID (\d+), name "([^"]+)", type "([^"]+)"\)$/)) {
						generator.end_attrs << TrAttrType.getAttrType(matcher[0][2], matcher[0][3], ITrAttrType.AttributeType.END)
					}
					break;
				case ")":
					generator=null
					break
				case "tx_begin"://matcher = line =~ /^tx_begin\s+(\d+)\s+(\d+)\s+(\d+)\s+([munpf]?s)/
					def id = Integer.parseInt(tokens[1])
					TrGenerator gen=generatorsById[Integer.parseInt(tokens[2])]
					transaction = new Transaction(id, gen.stream, gen, new EventTime(Integer.parseInt(tokens[3]), tokens[4]))
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
					transactionsById[id].attributes<<new TrAttribute(tokens[2][1..-2], tokens[3], ITrAttrType.AttributeType.UNSPECIFIED, tokens[5..-1].join(' '))
					break
				case "a"://matcher = line =~ /^a\s+(.+)$/
					if(endTransaction){
						transaction.attributes << new TrAttribute(transaction.generator.end_attrs[0], tokens[1])
					} else {
						transaction.attributes << new TrAttribute(transaction.generator.begin_attrs[0], tokens[1])
					}
					break
				case "tx_relation"://matcher = line =~ /^tx_relation\s+\"(\S+)\"\s+(\d+)\s+(\d+)$/
					Transaction tr1= transactionsById[Integer.parseInt(tokens[2])]
					Transaction tr2= transactionsById[Integer.parseInt(tokens[3])]
					def relType=tokens[1][1..-2]
					if(!relationTypes.containsKey(relType)) relationTypes[relType]=new RelationType(relType)
					def rel = new TrRelation(relationTypes[relType], tr1, tr2)
					tr1.outgoingRelations<<rel
					tr2.incomingRelations<<rel
					break
				default:
					println "Don't know what to do with: '$line'"

			}
		}
		addHierarchyNodes()
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
