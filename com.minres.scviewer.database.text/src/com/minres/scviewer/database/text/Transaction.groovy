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

import java.util.Collection;
import java.util.Set

import com.minres.scviewer.database.*

class Transaction implements ITransaction {
	
	Long id
	
	Generator generator

	Stream stream
	
	EventTime beginTime
	
	EventTime endTime
	
	ArrayList<ITrAttribute> attributes = new ArrayList<ITrAttribute>()
	
	def incomingRelations =[]
	
	def outgoingRelations =[]
	
	Transaction(int id, Stream stream, Generator generator, EventTime begin){
		this.id=id
		this.generator=generator
		this.beginTime=begin
	}
	
	@Override
	public Collection<ITrRelation> getIncomingRelations() {
		return incomingRelations;
	}

	@Override
	public Collection<ITrRelation> getOutgoingRelations() {
		return outgoingRelations;
	}
	
}
