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
package com.minres.scviewer.e4.application.parts;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.annotation.PostConstruct;
import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.core.runtime.SubMonitor;
import org.eclipse.core.runtime.jobs.IJobChangeEvent;
import org.eclipse.core.runtime.jobs.Job;
import org.eclipse.core.runtime.jobs.JobChangeAdapter;
import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.di.Focus;
import org.eclipse.e4.ui.di.PersistState;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.services.EMenuService;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.swt.widgets.Composite;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformDbFactory;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.swt.WaveformViewerFactory;
import com.minres.scviewer.database.ui.GotoDirection;
import com.minres.scviewer.database.ui.IWaveformViewer;
import com.minres.scviewer.database.ui.TrackEntry;
import com.minres.scviewer.e4.application.internal.WaveStatusBarControl;

public class WaveformViewerPart {

	public static final String ACTIVE_WAVEFORMVIEW="Active_Waveform_View";
	
	public static final String ADD_WAVEFORM="AddWaveform";

	protected static final String DATABASE_FILE = "DATABASE_FILE";

	protected static final String SHOWN_WAVEFORM = "SHOWN_WAVEFORM";

	private String[] zoomLevel;

	public static final String ID = "com.minres.scviewer.ui.TxEditorPart"; //$NON-NLS-1$

	public static final String WAVE_ACTION_ID = "com.minres.scviewer.ui.action.AddToWave";

	WaveformViewerFactory factory = new WaveformViewerFactory();

	private IWaveformViewer waveformPane;

	@Inject	private IEventBroker eventBroker;

	@Inject EMenuService menuService;

	@Inject	ESelectionService selectionService;

	private IWaveformDb database;

	private Composite myParent;

	ArrayList<File> filesToLoad;

	Map<String, String> persistedState;


	@PostConstruct
	public void createComposite(MPart part, Composite parent, IWaveformDbFactory dbFactory) {
		myParent=parent;	
		database=dbFactory.getDatabase();
		database.addPropertyChangeListener(new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				if("WAVEFORMS".equals(evt.getPropertyName())) {
					myParent.getDisplay().syncExec(new Runnable() {						
						@Override
						public void run() {
							waveformPane.setMaxTime(database.getMaxTime());
						}
					});
				}		
			}
		});
		waveformPane = factory.createPanel(parent);
		waveformPane.setMaxTime(0);
		waveformPane.addPropertyChangeListener(IWaveformViewer.CURSOR_PROPERTY, new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				Long time = (Long) evt.getNewValue();
				eventBroker.post(WaveStatusBarControl.CURSOR_TIME, waveformPane.getScaledTime(time));
				long marker=waveformPane.getActMarkerTime();
				eventBroker.post(WaveStatusBarControl.MARKER_DIFF, waveformPane.getScaledTime(time-marker));

			}
		});
		waveformPane.addPropertyChangeListener(IWaveformViewer.MARKER_PROPERTY, new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				Long time = (Long) evt.getNewValue();
				eventBroker.post(WaveStatusBarControl.MARKER_TIME, waveformPane.getScaledTime(time));
				long cursor=waveformPane.getCursorTime();
				eventBroker.post(WaveStatusBarControl.MARKER_DIFF, waveformPane.getScaledTime(cursor-time));
			}
		});
		waveformPane.addSelectionChangedListener(new ISelectionChangedListener() {
			@Override
			public void selectionChanged(SelectionChangedEvent event) {
				if(event.getSelection() instanceof IStructuredSelection)
					selectionService.setSelection(event.getSelection());
			}
		});
		zoomLevel=waveformPane.getZoomLevels();
		filesToLoad=new ArrayList<File>();
		persistedState = part.getPersistedState();
		Integer files = persistedState.containsKey(DATABASE_FILE+"S")?Integer.parseInt(persistedState.get(DATABASE_FILE+"S")):0;
		for(int i=0; i<files;i++){
			filesToLoad.add(new File(persistedState.get(DATABASE_FILE+i)));
		}
		if(filesToLoad.size()>0)
			loadDatabase();
		eventBroker.post(WaveStatusBarControl.ZOOM_LEVEL, zoomLevel[waveformPane.getZoomLevel()]);
		menuService.registerContextMenu(waveformPane.getNameControl(), "com.minres.scviewer.e4.application.popupmenu.namecontext");
		menuService.registerContextMenu(waveformPane.getValueControl(), "com.minres.scviewer.e4.application.popupmenu.namecontext");
		menuService.registerContextMenu(waveformPane.getWaveformControl(), "com.minres.scviewer.e4.application.popupmenu.wavecontext");
	}

	protected void loadDatabase() {
		Job job = new Job(" My Job") {
			@Override
			protected IStatus run( IProgressMonitor monitor) {
				// convert to SubMonitor and set total number of work units
				SubMonitor subMonitor = SubMonitor.convert(monitor, filesToLoad.size());
				subMonitor.setTaskName("Loading database");
				try {
					for(File file: filesToLoad){
						//TimeUnit.SECONDS.sleep(2);
						database.load(file);
						database.addPropertyChangeListener(waveformPane);
						subMonitor.worked(1);
						if(monitor.isCanceled()) return Status.CANCEL_STATUS;
					}
					// sleep a second
				} catch (Exception e) {
					database=null;
					e.printStackTrace();
					return Status.CANCEL_STATUS;
				}
				subMonitor.done();
				monitor.done();
				return Status.OK_STATUS;
			}
		};
		job.addJobChangeListener(new JobChangeAdapter(){
			@Override
			public void done(IJobChangeEvent event) {
				if(event.getResult()==Status.OK_STATUS)
					myParent.getDisplay().asyncExec(new Runnable() {						
						@Override
						public void run() {
							waveformPane.setMaxTime(database.getMaxTime());
							restoreState();
						}
					});
			}
		});
		job.schedule(0);
	}

	@Inject
	@Optional
	public void setPartInput( @Named( "input" ) Object partInput ) {
		if(partInput instanceof File){
			filesToLoad=new ArrayList<File>();
			File file = (File) partInput;
			if(file.exists()){
				filesToLoad.add(file);
				try {
					String ext = getFileExtension(file.getName());
					if("vcd".equals(ext.toLowerCase())){
						if(askIfToLoad(new File(renameFileExtension(file.getCanonicalPath(), "txdb")))){
							filesToLoad.add(new File(renameFileExtension(file.getCanonicalPath(), "txdb")));
						}else if(askIfToLoad(new File(renameFileExtension(file.getCanonicalPath(), "txlog")))){
							filesToLoad.add(new File(renameFileExtension(file.getCanonicalPath(), "txlog")));
						}
					} else if("txdb".equals(ext.toLowerCase()) || "txlog".equals(ext.toLowerCase())){
						if(askIfToLoad(new File(renameFileExtension(file.getCanonicalPath(), "vcd")))){
							filesToLoad.add(new File(renameFileExtension(file.getCanonicalPath(), "vcd")));
						}
					}
				} catch (IOException e) { // silently ignore any error
				}
			}
			if(filesToLoad.size()>0)
				loadDatabase();
		}
	}

	@Focus
	public void setFocus() {
		myParent.setFocus();
		updateAll();
	}

	@PersistState
	public void saveState(MPart part) {
		// save changes 
		Map<String, String> persistedState = part.getPersistedState();
		persistedState.put(DATABASE_FILE+"S", Integer.toString(filesToLoad.size()));
		Integer index=0;
		for(File file:filesToLoad){
			persistedState.put(DATABASE_FILE+index, file.getAbsolutePath());
			index++;
		}
		persistedState.put(SHOWN_WAVEFORM+"S", Integer.toString(waveformPane.getStreamList().size()));
		index=0;
		for(TrackEntry trackEntry:waveformPane.getStreamList()){
			persistedState.put(SHOWN_WAVEFORM+index, trackEntry.waveform.getFullName());
			index++;
		}
	} 

	protected void restoreState() {
		updateAll();
		Integer waves = persistedState.containsKey(SHOWN_WAVEFORM+"S")?Integer.parseInt(persistedState.get(SHOWN_WAVEFORM+"S")):0;
		List<TrackEntry> res = new LinkedList<>();
		for(int i=0; i<waves;i++){
			IWaveform<? extends IWaveformEvent> waveform = database.getStreamByName(persistedState.get(SHOWN_WAVEFORM+i));
			if(waveform!=null) res.add(new TrackEntry(waveform));
		}
		if(res.size()>0) waveformPane.getStreamList().addAll(res);
	}

	private void updateAll() {
		eventBroker.post(ACTIVE_WAVEFORMVIEW, this);
		eventBroker.post(WaveStatusBarControl.ZOOM_LEVEL, zoomLevel[waveformPane.getZoomLevel()]);
		long cursor=waveformPane.getCursorTime();
		long marker=waveformPane.getActMarkerTime();
		eventBroker.post(WaveStatusBarControl.CURSOR_TIME, waveformPane.getScaledTime(cursor));
		eventBroker.post(WaveStatusBarControl.MARKER_TIME, waveformPane.getScaledTime(marker));
		eventBroker.post(WaveStatusBarControl.MARKER_DIFF, waveformPane.getScaledTime(cursor-marker));
	}

	@Inject @Optional
	public void  getAddWaveformEvent(@UIEventTopic(WaveformViewerPart.ADD_WAVEFORM) Object o) {
		Object sel = o==null?selectionService.getSelection():o;
		if(sel instanceof IStructuredSelection)
			for(Object el:((IStructuredSelection)sel).toArray()){
				if(el instanceof IWaveform<?>) 
					addStreamToList((IWaveform<?>) el, false);
			}
	}

	/*	
    @Inject 
	public void setWaveform(@Optional @Named( IServiceConstants.ACTIVE_SELECTION) IWaveform<?> waveform, 
			@Optional @Named( IServiceConstants.ACTIVE_PART) MPart part) {
		if (txDisplay!= null && part.getObject()!=this) {
			txDisplay.setSelection(waveform==null?new StructuredSelection():new StructuredSelection(waveform)); 
		}
	}
	 */
	protected boolean askIfToLoad(File txFile) {
		if(txFile.exists() &&
				MessageDialog.openQuestion(myParent.getDisplay().getActiveShell(), "Database open", 
						"Would you like to open the adjacent database "+txFile.getName()+" as well?")){
			return true;
		}
		return false;
	}

	protected static String renameFileExtension(String source, String newExt) {
		String target;
		String currentExt = getFileExtension(source);
		if (currentExt.equals("")){
			target=source+"."+newExt;
		} else {
			target=source.replaceFirst(Pattern.quote("."+currentExt)+"$", Matcher.quoteReplacement("."+newExt));
		}
		return target;
	}

	protected static String getFileExtension(String f) {
		String ext = "";
		int i = f.lastIndexOf('.');
		if (i > 0 &&  i < f.length() - 1) {
			ext = f.substring(i + 1);
		}
		return ext;
	}

	public IWaveformDb getModel() {
		return database;
	}

	public IWaveformDb getDatabase() {
		return database;
	}

	public void addStreamToList(IWaveform<? extends IWaveformEvent> obj, boolean insert){
		addStreamsToList(new IWaveform<?>[]{obj}, insert);
	}

	public void addStreamsToList(IWaveform<? extends IWaveformEvent>[] iWaveforms, boolean insert){
		List<TrackEntry> streams= new LinkedList<>();
		for(IWaveform<? extends IWaveformEvent> stream:iWaveforms)
			streams.add(new TrackEntry(stream));
		IStructuredSelection selection = (IStructuredSelection) waveformPane.getSelection();
		if(selection.size()==0){
			waveformPane.getStreamList().addAll(streams);
		}else {
			Object first=selection.getFirstElement();
			IWaveform<?> stream = (first instanceof ITx)?((ITx)first).getStream():(IWaveform<?>)first;
			TrackEntry trackEntry=waveformPane.getEntryForStream(stream);
			int index = waveformPane.getStreamList().indexOf(trackEntry);
			if(!insert) index++;
			waveformPane.getStreamList().addAll(index, streams);
		}
	}

	public void removeStreamFromList(IWaveform<? extends IWaveformEvent> stream){
		TrackEntry trackEntry=waveformPane.getEntryForStream(stream);
		waveformPane.getStreamList().remove(trackEntry);
	}

	public void removeStreamsFromList(IWaveform<? extends IWaveformEvent>[] iWaveforms){
		for(IWaveform<? extends IWaveformEvent> stream:iWaveforms)
			removeStreamFromList(stream);
	}

	public void moveSelected(int i) {
		waveformPane.moveSelected(i);
	}

	public void moveSelection(GotoDirection direction) {
		waveformPane.moveSelection(direction);
	}

	public void moveCursor(GotoDirection direction) {
		waveformPane.moveCursor(direction);	}

	public void setZoomLevel(Integer level) {
		if(level<0) level=0;
		if(level>zoomLevel.length-1) level=zoomLevel.length-1;
		waveformPane.setZoomLevel(level);
		updateAll();	}

	public void setZoomFit() {
		waveformPane.setZoomLevel(6);		
		updateAll();	}

	public int getZoomLevel() {
		return waveformPane.getZoomLevel();
	}

	public ISelection getSelection() {
		return waveformPane.getSelection();
	}

	public void setSelection(IStructuredSelection structuredSelection) {
		waveformPane.setSelection(structuredSelection, true);
	}

	public String getScaledTime(Long time) {
		return waveformPane.getScaledTime(time);
	}
}
