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

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.IPropertySource;
import org.eclipse.ui.views.properties.PropertyDescriptor;

import com.itjw.txviewer.database.ITrHierNode;
import com.itjw.txviewer.database.ITrStream;

public class ITrHierNodeFacade implements ITrHierNode, IPropertySource {
	protected final static String PROPERTY_NAME = "Name";
	protected final static String PROPERTY_FULLNAME = "HierName";
	
	private transient PropertyChangeSupport pcs = new PropertyChangeSupport(this);

	protected ITrHierNode iTrHierNode;

	protected ITrHierNodeFacade parent;
	
	protected IPropertyDescriptor[] propertyDescriptors;
    
    public ITrHierNodeFacade(ITrHierNode iTrHierNode) {
		this.iTrHierNode = iTrHierNode;
	}

	public ITrHierNodeFacade(ITrHierNode iTrHierNode, ITrHierNodeFacade parent) {
		this.iTrHierNode = iTrHierNode;
		this.parent=parent;
	}

	@Override
	public String getFullName() {
		return iTrHierNode.getFullName();
	}

	@Override
	public void setName(String name) {
		iTrHierNode.setName(name);
	}

	@Override
	public String getName() {
		return iTrHierNode.getName();
	}

	@Override
	public List<ITrHierNode> getChildNodes() {
		ArrayList<ITrHierNode> res = new ArrayList<ITrHierNode>();
		for(ITrHierNode node:iTrHierNode.getChildNodes()){
			if(node instanceof ITrStream){
				res.add(new ITrStreamFacade((ITrStream)node));
			} else {
				res.add(new ITrHierNodeFacade(node));
			}
		}
		return res;
	}

	/**
	 * Attach a non-null PropertyChangeListener to this object.
	 * 
	 * @param l
	 *            a non-null PropertyChangeListener instance
	 * @throws IllegalArgumentException
	 *             if the parameter is null
	 */
	public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
		if (l == null) {
			throw new IllegalArgumentException();
		}
		pcs.addPropertyChangeListener(l);
	}
	/**
	 * Report a property change to registered listeners (for example edit
	 * parts).
	 * 
	 * @param property
	 *            the programmatic name of the property that changed
	 * @param oldValue
	 *            the old value of this property
	 * @param newValue
	 *            the new value of this property
	 */
	protected void firePropertyChange(String property, Object oldValue,	Object newValue) {
		if (pcs.hasListeners(property)) {
			pcs.firePropertyChange(property, oldValue, newValue);
		}
	}

	/**
	 * Remove a PropertyChangeListener from this component.
	 * 
	 * @param l
	 *            a PropertyChangeListener instance
	 */
	public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
		if (l != null) {
			pcs.removePropertyChangeListener(l);
		}
	}

	// IPropertySource methods
	@Override
	public Object getEditableValue() {
		return null;
	}

	@Override
	public IPropertyDescriptor[] getPropertyDescriptors() {
        if (propertyDescriptors == null) {
            // Create a descriptor and set a category
            PropertyDescriptor nameDescriptor = new PropertyDescriptor(PROPERTY_NAME, "Name");
            nameDescriptor.setCategory("Hier Node");
            PropertyDescriptor fullnameDescriptor = new PropertyDescriptor(PROPERTY_FULLNAME, "Full name");
            fullnameDescriptor.setCategory("Hier Node");
            propertyDescriptors = new IPropertyDescriptor[] {nameDescriptor, fullnameDescriptor};
        }
        return propertyDescriptors;
	}

	@Override
	public Object getPropertyValue(Object id) {
		if (id.equals(PROPERTY_NAME)) {
			return getName();
		} else if(id.equals(PROPERTY_FULLNAME)){
			return getFullName();
		} else
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
