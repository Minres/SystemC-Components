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
package com.itjw.txviewer.graph.data;

import java.util.List;
import java.util.Set;

import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.ui.views.properties.IPropertySource;

import com.itjw.txviewer.database.EventTime;
import com.itjw.txviewer.database.ITrAttribute;
import com.itjw.txviewer.database.ITrGenerator;
import com.itjw.txviewer.database.ITransaction;
import com.itjw.txviewer.database.RelationType;

public class ITransactionFacade implements ITransaction, IAdaptable {

	protected ITransaction iTransaction;
    
    public ITransactionFacade(ITransaction iTransaction) {
		this.iTransaction = iTransaction;
	}
    
	@Override
	public Long getId() {
		return iTransaction.getId();
	}

	@Override
	public ITrGenerator getGenerator() {
		return iTransaction.getGenerator();
	}

	@Override
	public EventTime getBeginTime() {
		return iTransaction.getBeginTime();
	}

	@Override
	public EventTime getEndTime() {
		return iTransaction.getEndTime();
	}

	@Override
	public List<ITrAttribute> getBeginAttrs() {
		return iTransaction.getBeginAttrs();
	}

	@Override
	public List<ITrAttribute> getEndAttrs() {
		return iTransaction.getEndAttrs();
	}

	@Override
	public List<ITrAttribute> getAttributes() {
		return iTransaction.getAttributes();
	}

	@Override
	public Set<ITransaction> getNextInRelationship(RelationType rel) {
		return iTransaction.getNextInRelationship(rel);
	}

	@SuppressWarnings("rawtypes")
	@Override
	public Object getAdapter(Class adapter) {
		if (adapter == IPropertySource.class)
			return new ITransactionProperties(this);
		return null;
	}
}
