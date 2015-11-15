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
package com.minres.scviewer.database.text;

import java.util.Collection;

import com.minres.scviewer.database.AssociationType
import com.minres.scviewer.database.DataType
import com.minres.scviewer.database.ITxGenerator
import com.minres.scviewer.database.ITxStream
import com.minres.scviewer.database.IWaveform
import com.minres.scviewer.database.IWaveformDb
import com.minres.scviewer.database.IWaveformDbLoader
import com.minres.scviewer.database.RelationType

public class TextDbLoader implements IWaveformDbLoader{

	private Long maxTime;

	IWaveformDb db;
		
	def streams = []
	
	def relationTypes=[:]
	
	public TextDbLoader() {
	}

	@Override
	public Long getMaxTime() {
		return maxTime;
	}

	@Override
	public List<IWaveform> getAllWaves() {
		return new LinkedList<IWaveform>(streams);
	}

	public Map<Long, ITxGenerator> getGeneratorsById() {
		TreeMap<Long, ITxGenerator> res = new TreeMap<Long, ITxGenerator>();
		streams.each{TxStream stream -> stream.generators.each{res.put(it.id, id)} }
		return res;
	}

	static final byte[] x = "scv_tr_stream".bytes

	@Override
	boolean load(IWaveformDb db, File file) throws Exception {
		this.db=db
		this.streams=[]
		FileInputStream fis = new FileInputStream(file)
        byte[] buffer = new byte[x.size()]
        def readCnt = fis.read(buffer, 0, x.size())
        fis.close()
        if(readCnt==x.size())
        	for(int i=0; i<x.size(); i++)
        		if(buffer[i]!=x[i]) return false
		parseInput(file)
		calculateConcurrencyIndicees()
		return true
	}

	private stringToScale(String scale){
		switch(scale.trim()){
			case "fs":return 1L
			case "ps":return 1000L
			case "ns":return 1000000L
			case "us":return 1000000000L
			case "ms":return 1000000000000L
			case "s": return 1000000000000000L
		}
	}
	private def parseInput(File input){
		def streamsById = [:]
		def generatorsById = [:]
		def transactionsById = [:]
		TxGenerator generator
		Tx transaction
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
						def stream = new TxStream(db, id, matcher[0][2], matcher[0][3])
						streams<<stream
						streamsById[id]=stream
					} else if ((matcher = line =~ /^scv_tr_generator\s+\(ID\s+(\d+),\s+name\s+"([^"]+)",\s+scv_tr_stream\s+(\d+),$/)) {
						def id = Integer.parseInt(matcher[0][1])
						ITxStream stream=streamsById[Integer.parseInt(matcher[0][3])]
						generator=new TxGenerator(id, stream, matcher[0][2])
						stream.generators<<generator
						generatorsById[id]=generator
					} else if ((matcher = line =~ /^begin_attribute \(ID (\d+), name "([^"]+)", type "([^"]+)"\)$/)) {
						generator.begin_attrs << TxAttributeType.getAttrType(matcher[0][2], DataType.valueOf(matcher[0][3]), AssociationType.BEGIN)
					} else if ((matcher = line =~ /^end_attribute \(ID (\d+), name "([^"]+)", type "([^"]+)"\)$/)) {
						generator.end_attrs << TxAttributeType.getAttrType(matcher[0][2], DataType.valueOf(matcher[0][3]), AssociationType.END)
					}
					break;
				case ")":
					generator=null
					break
				case "tx_begin"://matcher = line =~ /^tx_begin\s+(\d+)\s+(\d+)\s+(\d+)\s+([munpf]?s)/
					def id = Integer.parseInt(tokens[1])
					TxGenerator gen=generatorsById[Integer.parseInt(tokens[2])]
					transaction = new Tx(id, gen.stream, gen, Long.parseLong(tokens[3])*stringToScale(tokens[4]))
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
					transaction.endTime = Long.parseLong(tokens[3])*stringToScale(tokens[4])
					transaction.generator.end_attrs_idx=0;
					maxTime = maxTime>transaction.endTime?maxTime:transaction.endTime
					endTransaction=true
					break
				case "tx_record_attribute"://matcher = line =~ /^tx_record_attribute\s+(\d+)\s+"([^"]+)"\s+(\S+)\s*=\s*(.+)$/
					def id = Integer.parseInt(tokens[1])
					transactionsById[id].attributes<<new TxAttribute(tokens[2][1..-2], DataType.valueOf(tokens[3]), AssociationType.RECORD, tokens[5..-1].join(' '))
					break
				case "a"://matcher = line =~ /^a\s+(.+)$/
					if(endTransaction){
						transaction.attributes << new TxAttribute(transaction.generator.end_attrs[0], tokens[1])
					} else {
						transaction.attributes << new TxAttribute(transaction.generator.begin_attrs[0], tokens[1])
					}
					break
				case "tx_relation"://matcher = line =~ /^tx_relation\s+\"(\S+)\"\s+(\d+)\s+(\d+)$/
					Tx tr2= transactionsById[Integer.parseInt(tokens[2])]
					Tx tr1= transactionsById[Integer.parseInt(tokens[3])]
					def relType=tokens[1][1..-2]
					if(!relationTypes.containsKey(relType)) relationTypes[relType]=RelationType.create(relType)
					def rel = new TxRelation(relationTypes[relType], tr1, tr2)
					tr1.outgoingRelations<<rel
					tr2.incomingRelations<<rel
					break
				default:
					println "Don't know what to do with: '$line'"

			}
		}
	}

	private def calculateConcurrencyIndicees(){
		streams.each{ TxStream  stream -> stream.getMaxConcurrency() }
	}
	
	
	public Collection<RelationType> getAllRelationTypes(){
		return relationTypes.values();
	}

}
