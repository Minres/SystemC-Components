package com.itjw.txviewer.database;

import java.beans.PropertyChangeListener;
import java.util.List;

public interface ITrHierNode {
	/**
	 * Attach a non-null PropertyChangeListener to this object.
	 * 
	 * @param l
	 *            a non-null PropertyChangeListener instance
	 * @throws IllegalArgumentException
	 *             if the parameter is null
	 */
	public void addPropertyChangeListener(PropertyChangeListener l);
	/**
	 * Remove a PropertyChangeListener from this component.
	 * 
	 * @param l
	 *            a PropertyChangeListener instance
	 */
	public void removePropertyChangeListener(PropertyChangeListener l) ;

	public String getFullName();
	
	public String getName();
	
	public void setName(String name);
	
	public List<ITrHierNode> getChildNodes();

}
