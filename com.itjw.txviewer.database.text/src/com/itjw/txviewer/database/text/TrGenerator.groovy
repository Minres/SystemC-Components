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
package com.itjw.txviewer.database.text

import java.util.ArrayList;
import java.util.List;

import com.itjw.txviewer.database.ITrAttrType
import com.itjw.txviewer.database.ITrAttribute;
import com.itjw.txviewer.database.ITrGenerator;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.ITransaction;

class TrGenerator implements ITrGenerator{
	Long id
	TrStream stream
	String name
	Boolean active = false
	ArrayList<ITransaction> transactions=[]
	
	ArrayList<ITrAttrType> begin_attrs = []
	int begin_attrs_idx = 0
	ArrayList<ITrAttrType> end_attrs= []
	int end_attrs_idx = 0
	
	TrGenerator(int id, TrStream stream, name){
		this.id=id
		this.stream=stream
		this.name=name
	}
	
	ITrStream getStream(){
		return stream;
	}
	
	List<ITransaction> getTransactions(){
		return transactions
	}
	
	Boolean isActive() {return active};
	
}
