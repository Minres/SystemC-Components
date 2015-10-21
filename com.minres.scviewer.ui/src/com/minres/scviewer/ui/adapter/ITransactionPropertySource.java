/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.ui.adapter;

import java.util.ArrayList;

import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.IPropertySource;
import org.eclipse.ui.views.properties.PropertyDescriptor;

import com.minres.scviewer.database.ITx;

public class ITransactionPropertySource implements IPropertySource {

	private ITx iTransaction;

	public final static String PROPERTY_ID = "ID";
	public final static String PROPERTY_BEGINTIME = "BEGINTIME";
	public final static String PROPERTY_ENDTIME = "ENDTIME";
	public final static String PROPERTY_NAME = "NAME";
	
	protected IPropertyDescriptor[] propertyDescriptors;

	public ITransactionPropertySource(ITx iTransaction) {
		this.iTransaction=iTransaction;
		}

	@Override
	public Object getEditableValue() {
		return null;
	}

	@Override
	public IPropertyDescriptor[] getPropertyDescriptors() {
        if (propertyDescriptors == null) {
        	ArrayList<IPropertyDescriptor> descriptor=new ArrayList<IPropertyDescriptor>();
            // Create a descriptor and set a category
            PropertyDescriptor idDescriptor = new PropertyDescriptor(PROPERTY_ID, "Id");
            idDescriptor.setCategory("Attributes");
            descriptor.add(idDescriptor);
            PropertyDescriptor begTimeDescriptor = new PropertyDescriptor(PROPERTY_BEGINTIME, "Begin time");
            begTimeDescriptor.setCategory("Attributes");
            descriptor.add(begTimeDescriptor);
            PropertyDescriptor endTimeDescriptor = new PropertyDescriptor(PROPERTY_ENDTIME, "End time");
            endTimeDescriptor.setCategory("Attributes");
            descriptor.add(endTimeDescriptor);
            PropertyDescriptor nameDescriptor = new PropertyDescriptor(PROPERTY_NAME, "Name");
            nameDescriptor.setCategory("Attributes");
            descriptor.add(nameDescriptor);
          propertyDescriptors = descriptor.toArray(new IPropertyDescriptor[descriptor.size()]);
        }
        return propertyDescriptors;
	}

	@Override
	public Object getPropertyValue(Object id) {
		if (id.equals(PROPERTY_ID)) {
			return iTransaction.getId();
		} else if(id.equals(PROPERTY_BEGINTIME)){
			return iTransaction.getBeginTime();//.getValueNS();
		} else if(id.equals(PROPERTY_ENDTIME)){
			return iTransaction.getEndTime();//.getValueNS();
		} else if(id.equals(PROPERTY_NAME)){
			return iTransaction.getGenerator().getName();
		}
		return null;
	}

	@Override
	public boolean isPropertySet(Object id) {
		String curName = (String)getPropertyValue(id);
		return curName!=null && curName.length()>0;
	}

	@Override
	public void resetPropertyValue(Object id) {
	}

	@Override
	public void setPropertyValue(Object id, Object value) {
	}

}
