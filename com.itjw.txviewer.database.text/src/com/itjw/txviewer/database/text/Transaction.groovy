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

import java.util.Set
import com.itjw.txviewer.database.*
import org.eclipse.ui.views.properties.IPropertyDescriptor
import org.eclipse.ui.views.properties.IPropertySource

class Transaction implements IPropertySource, ITransaction {
	Long id
	TrGenerator generator
	EventTime beginTime
	EventTime endTime
	ArrayList<ITrAttribute> begin_attrs = new ArrayList<ITrAttribute>()
	ArrayList<ITrAttribute> end_attrs = new ArrayList<ITrAttribute>()
	ArrayList<ITrAttribute> attributes = new ArrayList<ITrAttribute>()
	Transaction prev, next
	def pred =[]
	def succ =[]
	def parent =[]
	def child =[]
	
	Transaction(int id, TrGenerator generator, EventTime begin){
		this.id=id
		this.generator=generator
		this.beginTime=begin
	}

	@Override
	List<ITrAttribute> getBeginAttrs() {
		return begin_attrs
	}
	
	@Override
	List<ITrAttribute> getEndAttrs() {
		return end_attrs
	}
	
	List<ITrAttribute> getAttributes(){
		return attributes
	}
	
	@Override
	public Set<ITransaction> getNextInRelationship(RelationType rel) {
		switch(rel){
		case RelationType.PREDECESSOR:
			return pred
			break
		case RelationType.SUCCESSOR:
			return succ
			break
		case RelationType.PREVIOUS:
			return [prev]
			break
		case RelationType.NEXT:
			return[next]
			break
		case RelationType.PARENT:
			return parent
			break
		case RelationType.CHILD:
			return child
			break
		}
	}

	@Override
	public Object getEditableValue() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public IPropertyDescriptor[] getPropertyDescriptors() {
		return null;
	}

	@Override
	public Object getPropertyValue(Object id) {
		return null;
	}

	@Override
	public boolean isPropertySet(Object id) {
		return false;
	}

	@Override
	public void resetPropertyValue(Object id) {
	}

	@Override
	public void setPropertyValue(Object id, Object value) {
	}
}
