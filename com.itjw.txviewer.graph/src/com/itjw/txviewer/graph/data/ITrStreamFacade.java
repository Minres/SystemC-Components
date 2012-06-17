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

import java.util.Arrays;
import java.util.List;

import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.PropertyDescriptor;

import com.itjw.txviewer.database.ITrDb;
import com.itjw.txviewer.database.ITrGenerator;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.ITransaction;

public class ITrStreamFacade extends ITrHierNodeFacade implements ITrStream {

	protected final static String PROPERTY_NUMBER = "NR_OF_TRANSACTIONS";
	private int height;
	
	private ITrStream getStream(){
		return (ITrStream)iTrHierNode;
	}
	
	public ITrStreamFacade(ITrStream iTrStream) {
		super(iTrStream);
	}

	public ITrStreamFacade(ITrStream iTrStream, ITrHierNodeFacade parent) {
		super(iTrStream, parent);
	}
	
	@Override
	public Long getId() {
		return getStream().getId();
	}

	@Override
	public String getKind() {
		return getStream().getKind();
	}

	@Override
	public ITrDb getDb() {
		// TODO Auto-generated method stub
		return getStream().getDb();
	}

	@Override
	public List<ITrGenerator> getGenerators() {
		return getStream().getGenerators();
	}

	@Override
	public List<ITransaction> getTransactions() {
		return getStream().getTransactions();
	}

	@Override
	public int getMaxConcurrrentTx() {
		return getStream().getMaxConcurrrentTx();
	}

	@Override
	public IPropertyDescriptor[] getPropertyDescriptors() {
        if (propertyDescriptors == null) {
    		super.getPropertyDescriptors();
            // Create a descriptor and set a category
            PropertyDescriptor nrDescriptor = new PropertyDescriptor(PROPERTY_NUMBER, "# of transactions");
            nrDescriptor.setCategory("Stream");
            IPropertyDescriptor[] result = Arrays.copyOf(propertyDescriptors, propertyDescriptors.length + 1);
            System.arraycopy(new IPropertyDescriptor[] {nrDescriptor}, 0, result, propertyDescriptors.length, 1);
            propertyDescriptors = result;
        }
        return propertyDescriptors;
	}

	@Override
	public Object getPropertyValue(Object id) {
		if (id.equals(PROPERTY_NUMBER)) {
			return getStream().getTransactions().size();
		} else {
			return super.getPropertyValue(id);
		}
	}

	public int getHeight() {
		return height;
	}

	public void setHeight(int height) {
		this.height = height;
	}


	public double getMaxTimeNS() {
		return getDb().getMaxTime().getValueNS();
	}

	public String getValueAtCursor(int i) {
		return "-";
	}

}
