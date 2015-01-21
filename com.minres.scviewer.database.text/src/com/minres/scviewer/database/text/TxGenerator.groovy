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

import java.util.ArrayList;
import java.util.List;

import com.minres.scviewer.database.ITxAttributeType
import com.minres.scviewer.database.ITxAttribute;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxGenerator;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveformEvent;

class TxGenerator implements ITxGenerator{
	Long id
	TxStream stream
	String name
	Boolean active = false
	ArrayList<ITx> transactions=[]
	
	ArrayList<ITxAttributeType> begin_attrs = []
	int begin_attrs_idx = 0
	ArrayList<ITxAttributeType> end_attrs= []
	int end_attrs_idx = 0
	
	TxGenerator(int id, TxStream stream, name){
		this.id=id
		this.stream=stream
		this.name=name
	}
	
	ITxStream<? extends ITxEvent> getStream(){
		return stream;
	}
	
	List<ITx> getTransactions(){
		return transactions
	}
	
	Boolean isActive() {return active};
	
}
