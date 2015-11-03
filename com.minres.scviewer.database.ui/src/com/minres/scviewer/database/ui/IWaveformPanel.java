package com.minres.scviewer.database.ui;

import java.beans.PropertyChangeListener;
import java.util.List;

import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.swt.widgets.Control;

import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;

public interface IWaveformPanel extends PropertyChangeListener, ISelectionProvider{

	String CURSOR_PROPERTY = "cursor_time";
	String MARKER_PROPERTY = "marker_time";

	void addSelectionChangedListener(ISelectionChangedListener listener);

	void removeSelectionChangedListener(ISelectionChangedListener listener);

	Control getControl();

	Control getNameControl();

	Control getValueControl();

	Control getWaveformControl();

	ISelection getSelection();

	void setSelection(ISelection selection);

	void setSelection(ISelection selection, boolean addIfNeeded);

	void moveSelection(GotoDirection direction);

	void moveCursor(GotoDirection direction);

	List<IWaveform<? extends IWaveformEvent>> getStreamList();

	void moveSelected(int i);

	long getMaxTime();

	void setMaxTime(long maxTime);

	void setZoomLevel(int scale);

	int getZoomLevel();

	void setCursorTime(long time);

	void setMarkerTime(long time, int index);

	long getCursorTime();

	long getActMarkerTime();

	long getMarkerTime(int index);

	void addPropertyChangeListener(PropertyChangeListener listener);

	void addPropertyChangeListener(String propertyName, PropertyChangeListener listener);

	void removePropertyChangeListener(PropertyChangeListener listener);

	void removePropertyChangeListener(String propertyName, PropertyChangeListener listener);

	String getScaledTime(long time);

	String[] getZoomLevels();

}