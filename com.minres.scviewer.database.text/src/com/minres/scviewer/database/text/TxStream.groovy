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
package com.minres.scviewer.database.text

import java.beans.PropertyChangeListener;
import java.util.List;
import java.util.Map;

import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.IWaveformDb
import com.minres.scviewer.database.ITxGenerator
import com.minres.scviewer.database.IHierNode
import com.minres.scviewer.database.ITxStream
import com.minres.scviewer.database.ITx

class TxStream extends HierNode implements ITxStream {

	Long id;
	
	TextDb database
	
	String fullName;
	
	String kind;
	
	def generators = [];
	
	private TreeSet<Tx> allTransactions;

	TxStream(int id, TextDb db, String name, String kind){
		super(name)
		this.id=id
		this.database=db
		this.fullName=name
		this.kind=kind
	}

	List<ITxGenerator> getGenerators(){
		return generators as List<ITxGenerator>
	}

	@Override
	public IWaveformDb getDb() {
		return database;
	}

	// FIXME: maybe need to be somewhere else
	public int getMaxConcurrrentTx() {
		def rowendtime = [0]
		getTransactions().each{Tx tx ->
			def rowIdx = 0
			for(rowIdx=0; rowendtime.size()<rowIdx || rowendtime[rowIdx]>tx.beginTime.value; rowIdx++);
				if(rowendtime.size<=rowIdx){
					rowendtime<<tx.endTime?.value?:tx.beginTime.value+1
				} else {
					rowendtime[rowIdx]=tx.endTime?.value?:tx.beginTime.value+1
				}
		}
		return rowendtime.size()
	}

	@Override
	public NavigableSet<ITx> getTransactions() {
		if(!allTransactions){
			allTransactions=new TreeSet<Tx>()
			allTransactions.addAll(generators.transactions.flatten())
		}
		return allTransactions
	}

	@Override
	public ITx getTransactionById(long id) {
		if(!allTransactions)
			allTransactions=generators.transactions.flatten().sort{it.beginTime.value}
		allTransactions.find{it.id==id}
	}
}
