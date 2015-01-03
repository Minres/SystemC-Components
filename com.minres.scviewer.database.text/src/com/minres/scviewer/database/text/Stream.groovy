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

import com.minres.scviewer.database.ITrDb
import com.minres.scviewer.database.ITrGenerator
import com.minres.scviewer.database.ITrHierNode
import com.minres.scviewer.database.ITrStream
import com.minres.scviewer.database.ITransaction

class Stream extends HierNode implements ITrStream {

	Long id;
	
	TextDb database
	
	String fullName;
	
	String kind;
	
	def generators = [];
	
	private allTransactions;

	public TrHierNode(String name){
		this.name=name
	}

	Stream(int id, TextDb db, String name, String kind){
		this.id=id
		this.database=db
		this.name=name
		this.fullName=name
		this.kind=kind
	}

	List<ITrGenerator> getGenerators(){
		return generators as List<ITrGenerator>
	}

	@Override
	public ITrDb getDb() {
		return database;
	}

	// FIXME: maybe need to be somewhere else
	public int getMaxConcurrrentTx() {
		def rowendtime = [0]
		getTransactions().each{Transaction tx ->
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
	public List<ITransaction> getTransactions() {
		if(!allTransactions)
			allTransactions=generators.transactions.flatten().sort{it.beginTime.value}
		return allTransactions
	}

	@Override
	public ITransaction getTransactionById(long id) {
		if(!allTransactions)
			allTransactions=generators.transactions.flatten().sort{it.beginTime.value}
		allTransactions.find{it.id==id}
	}
}
