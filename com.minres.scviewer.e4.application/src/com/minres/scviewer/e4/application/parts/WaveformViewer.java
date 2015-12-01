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
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Properties;
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
import org.eclipse.core.runtime.preferences.IEclipsePreferences;
import org.eclipse.core.runtime.preferences.IEclipsePreferences.IPreferenceChangeListener;
import org.eclipse.core.runtime.preferences.IEclipsePreferences.PreferenceChangeEvent;
import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.core.di.extensions.Preference;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.di.Focus;
import org.eclipse.e4.ui.di.PersistState;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.services.EMenuService;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.StringConverter;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxRelation;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformDbFactory;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.RelationType;
import com.minres.scviewer.database.swt.WaveformViewerFactory;
import com.minres.scviewer.database.ui.GotoDirection;
import com.minres.scviewer.database.ui.ICursor;
import com.minres.scviewer.database.ui.IWaveformViewer;
import com.minres.scviewer.database.ui.TrackEntry;
import com.minres.scviewer.database.ui.WaveformColors;
import com.minres.scviewer.e4.application.internal.status.WaveStatusBarControl;
import com.minres.scviewer.e4.application.internal.util.FileMonitor;
import com.minres.scviewer.e4.application.internal.util.IFileChangeListener;
import com.minres.scviewer.e4.application.internal.util.IModificationChecker;
import com.minres.scviewer.e4.application.preferences.DefaultValuesInitializer;
import com.minres.scviewer.e4.application.preferences.PreferenceConstants;

/**
 * The Class WaveformViewerPart.
 */
@SuppressWarnings("restriction")
public class WaveformViewer implements IFileChangeListener, IPreferenceChangeListener {

	/** The Constant ACTIVE_WAVEFORMVIEW. */
	public static final String ACTIVE_WAVEFORMVIEW = "Active_Waveform_View";

	/** The Constant ADD_WAVEFORM. */
	public static final String ADD_WAVEFORM = "AddWaveform";

	/** The Constant DATABASE_FILE. */
	protected static final String DATABASE_FILE = "DATABASE_FILE";
	
	/** The Constant SHOWN_WAVEFORM. */
	protected static final String SHOWN_WAVEFORM = "SHOWN_WAVEFORM";
	
	/** The Constant SHOWN_CURSOR. */
	protected static final String SHOWN_CURSOR = "SHOWN_CURSOR";
	
	/** The Constant ZOOM_LEVEL. */
	protected static final String ZOOM_LEVEL = "ZOOM_LEVEL";

	/** The Constant BASE_LINE_TIME. */
	protected static final String BASE_LINE_TIME = "BASE_LINE_TIME";

	/** The Constant FILE_CHECK_INTERVAL. */
	protected static final long FILE_CHECK_INTERVAL = 60000;
	
	/** The zoom level. */
	private String[] zoomLevel;

	/** The Constant ID. */
	public static final String ID = "com.minres.scviewer.ui.TxEditorPart"; //$NON-NLS-1$

	/** The Constant WAVE_ACTION_ID. */
	public static final String WAVE_ACTION_ID = "com.minres.scviewer.ui.action.AddToWave";

	/** The factory. */
	WaveformViewerFactory factory = new WaveformViewerFactory();

	/** The waveform pane. */
	private IWaveformViewer waveformPane;

	/** The event broker. */
	@Inject
	private IEventBroker eventBroker;

	/** The menu service. */
	@Inject
	EMenuService menuService;

	/** The selection service. */
	@Inject
	ESelectionService selectionService;

	/** The e part service. */
	@Inject
	EPartService ePartService;

	/** The prefs. */
	@Inject
	@Preference(nodePath = PreferenceConstants.PREFERENCES_SCOPE)
	IEclipsePreferences prefs;

	/** The database. */
	private IWaveformDb database;

	/** The check for updates. */
	private boolean checkForUpdates;

	/** The my part. */
	private MPart myPart;

	/** The my parent. */
	private Composite myParent;

	/** The files to load. */
	ArrayList<File> filesToLoad;

	/** The persisted state. */
	Map<String, String> persistedState;

	/** The browser state. */
	private Object browserState;

	/** The details settings. */
	private Object detailsSettings;

	/** The navigation relation type. */
	private RelationType navigationRelationType=IWaveformViewer.NEXT_PREV_IN_STREAM ;

	/** The file monitor. */
	FileMonitor fileMonitor = new FileMonitor();

	/** The file checker. */
	IModificationChecker fileChecker;

	/**
	 * Creates the composite.
	 *
	 * @param part the part
	 * @param parent the parent
	 * @param dbFactory the db factory
	 */
	@PostConstruct
	public void createComposite(MPart part, Composite parent, IWaveformDbFactory dbFactory) {
		myPart = part;
		myParent = parent;
		database = dbFactory.getDatabase();
		database.addPropertyChangeListener(new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				if ("WAVEFORMS".equals(evt.getPropertyName())) {
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
				long marker = waveformPane.getMarkerTime(waveformPane.getSelectedMarkerId());
				eventBroker.post(WaveStatusBarControl.MARKER_DIFF, waveformPane.getScaledTime(time - marker));

			}
		});
		waveformPane.addPropertyChangeListener(IWaveformViewer.MARKER_PROPERTY, new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				Long time = (Long) evt.getNewValue();
				eventBroker.post(WaveStatusBarControl.MARKER_TIME, waveformPane.getScaledTime(time));
				long cursor = waveformPane.getCursorTime();
				eventBroker.post(WaveStatusBarControl.MARKER_DIFF, waveformPane.getScaledTime(cursor - time));
			}
		});
		waveformPane.addSelectionChangedListener(new ISelectionChangedListener() {
			@Override
			public void selectionChanged(SelectionChangedEvent event) {
				if (event.getSelection() instanceof IStructuredSelection)
					selectionService.setSelection(event.getSelection());
			}
		});
		zoomLevel = waveformPane.getZoomLevels();
		setupColors();
		checkForUpdates = prefs.getBoolean(PreferenceConstants.DATABASE_RELOAD, true);
		filesToLoad = new ArrayList<File>();
		persistedState = part.getPersistedState();
		Integer files = persistedState.containsKey(DATABASE_FILE + "S")
				? Integer.parseInt(persistedState.get(DATABASE_FILE + "S")) : 0;
		for (int i = 0; i < files; i++) {
			filesToLoad.add(new File(persistedState.get(DATABASE_FILE + i)));
		}
		if (filesToLoad.size() > 0)
			loadDatabase(persistedState);
		eventBroker.post(WaveStatusBarControl.ZOOM_LEVEL, zoomLevel[waveformPane.getZoomLevel()]);
		menuService.registerContextMenu(waveformPane.getNameControl(),
				"com.minres.scviewer.e4.application.popupmenu.namecontext");
		menuService.registerContextMenu(waveformPane.getValueControl(),
				"com.minres.scviewer.e4.application.popupmenu.namecontext");
		menuService.registerContextMenu(waveformPane.getWaveformControl(),
				"com.minres.scviewer.e4.application.popupmenu.wavecontext");
		ePartService.addPartListener(new PartListener() {
			@Override
			public void partActivated(MPart part) {
				if (part == myPart) {
					if (fileChecker != null)
						fileChecker.check();
					updateAll();
				}
			}
		});
		prefs.addPreferenceChangeListener(this);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.core.runtime.preferences.IEclipsePreferences.IPreferenceChangeListener#preferenceChange(org.eclipse.core.runtime.preferences.IEclipsePreferences.PreferenceChangeEvent)
	 */
	@Override
	public void preferenceChange(PreferenceChangeEvent event) {
		if (PreferenceConstants.DATABASE_RELOAD.equals(event.getKey())) {
			checkForUpdates = (Boolean) event.getNewValue();
			fileChecker = null;
			if (checkForUpdates)
				fileChecker = fileMonitor.addFileChangeListener(WaveformViewer.this, filesToLoad,
						FILE_CHECK_INTERVAL);
			else
				fileMonitor.removeFileChangeListener(this);
		} else {
			setupColors();
		}
	}

	/**
	 * Setup colors.
	 */
	protected void setupColors() {
		DefaultValuesInitializer initializer = new DefaultValuesInitializer();
		HashMap<WaveformColors, RGB> colorPref = new HashMap<>();
		for (WaveformColors c : WaveformColors.values()) {
			String prefValue = prefs.get(c.name() + "_COLOR",
					StringConverter.asString(initializer.colors[c.ordinal()].getRGB()));
			RGB rgb = StringConverter.asRGB(prefValue);
			colorPref.put(c, rgb);
		}
		waveformPane.setColors(colorPref);
	}

	/**
	 * Load database.
	 *
	 * @param state the state
	 */
	protected void loadDatabase(final Map<String, String> state) {
		fileMonitor.removeFileChangeListener(this);
		Job job = new Job("Database Load Job") {
			@Override
			protected IStatus run(IProgressMonitor monitor) {
				// convert to SubMonitor and set total number of work units
				SubMonitor subMonitor = SubMonitor.convert(monitor, filesToLoad.size()+1);
				try {
					subMonitor.worked(1);
					for (File file : filesToLoad) {
						subMonitor.setTaskName("Loading "+file.getName());
						database.load(file);
						database.addPropertyChangeListener(waveformPane);
						subMonitor.worked(1);
						if (monitor.isCanceled())
							return Status.CANCEL_STATUS;
					}
				} catch (Exception e) {
					database = null;
					e.printStackTrace();
					return Status.CANCEL_STATUS;
				}
				subMonitor.done();
				monitor.done();
				return Status.OK_STATUS;
			}
		};
		job.addJobChangeListener(new JobChangeAdapter() {
			@Override
			public void done(IJobChangeEvent event) {
				if (event.getResult() == Status.OK_STATUS)
					myParent.getDisplay().asyncExec(new Runnable() {
						@Override
						public void run() {
							waveformPane.setMaxTime(database.getMaxTime());
							if (state != null)
								restoreWaveformViewerState(state);
							fileChecker = null;
							if (checkForUpdates)
								fileChecker = fileMonitor.addFileChangeListener(WaveformViewer.this, filesToLoad,
										FILE_CHECK_INTERVAL);
						}
					});
			}
		});
		job.schedule(0);
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.e4.application.internal.util.IFileChangeListener#fileChanged(java.util.List)
	 */
	@Override
	public void fileChanged(List<File> file) {
		final Display display = myParent.getDisplay();
		display.asyncExec(new Runnable() {
			@Override
			public void run() {
				if (MessageDialog.openQuestion(display.getActiveShell(), "Database re-load",
						"Would you like to reload the database?")) {
					Map<String, String> state = new HashMap<>();
					saveWaveformViewerState(state);
					waveformPane.getStreamList().clear();
					database.clear();
					if (filesToLoad.size() > 0)
						loadDatabase(state);
				}
			}
		});
		fileMonitor.removeFileChangeListener(this);
	}

	/**
	 * Sets the part input.
	 *
	 * @param partInput the new part input
	 */
	@Inject
	@Optional
	public void setPartInput(@Named("input") Object partInput) {
		if (partInput instanceof File) {
			filesToLoad = new ArrayList<File>();
			File file = (File) partInput;
			if (file.exists()) {
				filesToLoad.add(file);
				try {
					String ext = getFileExtension(file.getName());
					if ("vcd".equals(ext.toLowerCase())) {
						if (askIfToLoad(new File(renameFileExtension(file.getCanonicalPath(), "txdb")))) {
							filesToLoad.add(new File(renameFileExtension(file.getCanonicalPath(), "txdb")));
						} else if (askIfToLoad(new File(renameFileExtension(file.getCanonicalPath(), "txlog")))) {
							filesToLoad.add(new File(renameFileExtension(file.getCanonicalPath(), "txlog")));
						}
					} else if ("txdb".equals(ext.toLowerCase()) || "txlog".equals(ext.toLowerCase())) {
						if (askIfToLoad(new File(renameFileExtension(file.getCanonicalPath(), "vcd")))) {
							filesToLoad.add(new File(renameFileExtension(file.getCanonicalPath(), "vcd")));
						}
					}
				} catch (IOException e) { // silently ignore any error
				}
			}
			if (filesToLoad.size() > 0)
				loadDatabase(persistedState);
		}
	}

	/**
	 * Sets the focus.
	 */
	@Focus
	public void setFocus() {
		myParent.setFocus();
	}

	/**
	 * Save state.
	 *
	 * @param part the part
	 */
	@PersistState
	public void saveState(MPart part) {
		// save changes
		Map<String, String> persistedState = part.getPersistedState();
		persistedState.put(DATABASE_FILE + "S", Integer.toString(filesToLoad.size()));
		Integer index = 0;
		for (File file : filesToLoad) {
			persistedState.put(DATABASE_FILE + index, file.getAbsolutePath());
			index++;
		}
		saveWaveformViewerState(persistedState);
	}

	public void saveState(String fileName){
		Map<String, String> persistedState = new HashMap<>();
		persistedState.put(DATABASE_FILE + "S", Integer.toString(filesToLoad.size()));
		Integer index = 0;
		for (File file : filesToLoad) {
			persistedState.put(DATABASE_FILE + index, file.getAbsolutePath());
			index++;
		}
		saveWaveformViewerState(persistedState);
		Properties props = new Properties();
		props.putAll(persistedState);
		try {
			FileOutputStream out = new FileOutputStream(fileName);
			props.store(out, "Written by SCViewer");
			out.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void loadState(String fileName){
		Properties props = new Properties();
		try {
			FileInputStream in = new FileInputStream(fileName);
			props.load(in);
			in.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		@SuppressWarnings({ "unchecked", "rawtypes" })
		HashMap<String, String> propMap = new HashMap<String, String>((Map) props);
		restoreWaveformViewerState(propMap);
	}
	
	/**
	 * Save waveform viewer state.
	 *
	 * @param persistedState the persisted state
	 */
	protected void saveWaveformViewerState(Map<String, String> persistedState) {
		Integer index;
		persistedState.put(SHOWN_WAVEFORM + "S", Integer.toString(waveformPane.getStreamList().size()));
		index = 0;
		for (TrackEntry trackEntry : waveformPane.getStreamList()) {
			persistedState.put(SHOWN_WAVEFORM + index, trackEntry.waveform.getFullName());
			index++;
		}
		List<ICursor> cursors = waveformPane.getCursorList();
		persistedState.put(SHOWN_CURSOR + "S", Integer.toString(cursors.size()));
		index = 0;
		for (ICursor cursor : cursors) {
			persistedState.put(SHOWN_CURSOR + index, Long.toString(cursor.getTime()));
			index++;
		}
		persistedState.put(ZOOM_LEVEL, Integer.toString(waveformPane.getZoomLevel()));
		persistedState.put(BASE_LINE_TIME, Long.toString(waveformPane.getBaselineTime()));
	}

	/**
	 * Restore waveform viewer state.
	 *
	 * @param state the state
	 */
	protected void restoreWaveformViewerState(Map<String, String> state) {
		Integer waves = state.containsKey(SHOWN_WAVEFORM+"S") ? Integer.parseInt(state.get(SHOWN_WAVEFORM + "S")):0;
		List<TrackEntry> res = new LinkedList<>();
		for (int i = 0; i < waves; i++) {
			IWaveform<? extends IWaveformEvent> waveform = database.getStreamByName(state.get(SHOWN_WAVEFORM + i));
			if (waveform != null)
				res.add(new TrackEntry(waveform));
		}
		if (res.size() > 0)
			waveformPane.getStreamList().addAll(res);
		Integer cursorLength = state.containsKey(SHOWN_CURSOR+"S")?Integer.parseInt(state.get(SHOWN_CURSOR + "S")):0;
		List<ICursor> cursors = waveformPane.getCursorList();
		if (cursorLength == cursors.size()) {
			for (int i = 0; i < cursorLength; i++) {
				Long time = Long.parseLong(state.get(SHOWN_CURSOR + i));
				cursors.get(i).setTime(time);
			}
		}
		if (state.containsKey(ZOOM_LEVEL)) {
			try {
				Integer scale = Integer.parseInt(state.get(ZOOM_LEVEL));
				waveformPane.setZoomLevel(scale);
			} catch (NumberFormatException e) {
			}
		}
		if (state.containsKey(BASE_LINE_TIME)) {
			try {
				Long scale = Long.parseLong(state.get(BASE_LINE_TIME));
				waveformPane.setBaselineTime(scale);
			} catch (NumberFormatException e) {
			}
		}
		updateAll();
	}

	/**
	 * Update all status elements by posting respective events.
	 */
	private void updateAll() {
		eventBroker.post(ACTIVE_WAVEFORMVIEW, this);
		eventBroker.post(WaveStatusBarControl.ZOOM_LEVEL, zoomLevel[waveformPane.getZoomLevel()]);
		long cursor = waveformPane.getCursorTime();
		long marker = waveformPane.getMarkerTime(waveformPane.getSelectedMarkerId());
		eventBroker.post(WaveStatusBarControl.CURSOR_TIME, waveformPane.getScaledTime(cursor));
		eventBroker.post(WaveStatusBarControl.MARKER_TIME, waveformPane.getScaledTime(marker));
		eventBroker.post(WaveStatusBarControl.MARKER_DIFF, waveformPane.getScaledTime(cursor - marker));
	}

	/**
	 * Gets the adds the waveform event.
	 *
	 * @param o the o
	 * @return the adds the waveform event
	 */
	@Inject
	@Optional
	public void getAddWaveformEvent(@UIEventTopic(WaveformViewer.ADD_WAVEFORM) Object o) {
		Object sel = o == null ? selectionService.getSelection() : o;
		if (sel instanceof IStructuredSelection)
			for (Object el : ((IStructuredSelection) sel).toArray()) {
				if (el instanceof IWaveform<?>)
					addStreamToList((IWaveform<?>) el, false);
			}
	}

	/**
	 * Ask if to load.
	 *
	 * @param txFile the tx file
	 * @return true, if successful
	 */
	protected boolean askIfToLoad(File txFile) {
		if (txFile.exists() && MessageDialog.openQuestion(myParent.getDisplay().getActiveShell(), "Database open",
				"Would you like to open the adjacent database " + txFile.getName() + " as well?")) {
			return true;
		}
		return false;
	}

	/**
	 * Rename file extension.
	 *
	 * @param source the source
	 * @param newExt the new ext
	 * @return the string
	 */
	protected static String renameFileExtension(String source, String newExt) {
		String target;
		String currentExt = getFileExtension(source);
		if (currentExt.equals("")) {
			target = source + "." + newExt;
		} else {
			target = source.replaceFirst(Pattern.quote("." + currentExt) + "$", Matcher.quoteReplacement("." + newExt));
		}
		return target;
	}

	/**
	 * Gets the file extension.
	 *
	 * @param f the f
	 * @return the file extension
	 */
	protected static String getFileExtension(String f) {
		String ext = "";
		int i = f.lastIndexOf('.');
		if (i > 0 && i < f.length() - 1) {
			ext = f.substring(i + 1);
		}
		return ext;
	}

	/**
	 * Gets the model.
	 *
	 * @return the model
	 */
	public IWaveformDb getModel() {
		return database;
	}

	/**
	 * Gets the database.
	 *
	 * @return the database
	 */
	public IWaveformDb getDatabase() {
		return database;
	}

	/**
	 * Adds the stream to list.
	 *
	 * @param obj the obj
	 * @param insert the insert
	 */
	public void addStreamToList(IWaveform<? extends IWaveformEvent> obj, boolean insert) {
		addStreamsToList(new IWaveform<?>[] { obj }, insert);
	}

	/**
	 * Adds the streams to list.
	 *
	 * @param iWaveforms the i waveforms
	 * @param insert the insert
	 */
	public void addStreamsToList(IWaveform<? extends IWaveformEvent>[] iWaveforms, boolean insert) {
		List<TrackEntry> streams = new LinkedList<>();
		for (IWaveform<? extends IWaveformEvent> stream : iWaveforms)
			streams.add(new TrackEntry(stream));
		IStructuredSelection selection = (IStructuredSelection) waveformPane.getSelection();
		if (selection.size() == 0) {
			waveformPane.getStreamList().addAll(streams);
		} else {
			Object first = selection.getFirstElement();
			IWaveform<?> stream = (first instanceof ITx) ? ((ITx) first).getStream() : (IWaveform<?>) first;
			TrackEntry trackEntry = waveformPane.getEntryForStream(stream);
			int index = waveformPane.getStreamList().indexOf(trackEntry);
			if (!insert)
				index++;
			waveformPane.getStreamList().addAll(index, streams);
		}
	}

	/**
	 * Removes the stream from list.
	 *
	 * @param stream the stream
	 */
	public void removeStreamFromList(IWaveform<? extends IWaveformEvent> stream) {
		TrackEntry trackEntry = waveformPane.getEntryForStream(stream);
		waveformPane.getStreamList().remove(trackEntry);
	}

	/**
	 * Removes the streams from list.
	 *
	 * @param iWaveforms the i waveforms
	 */
	public void removeStreamsFromList(IWaveform<? extends IWaveformEvent>[] iWaveforms) {
		for (IWaveform<? extends IWaveformEvent> stream : iWaveforms)
			removeStreamFromList(stream);
	}

	/**
	 * Move selected.
	 *
	 * @param i the i
	 */
	public void moveSelected(int i) {
		waveformPane.moveSelectedTrack(i);
	}

	
	/**
	 * Move selection.
	 *
	 * @param direction the direction
	 */
	public void moveSelection(GotoDirection direction ) {
		moveSelection(direction, navigationRelationType); 
	}

	/**
	 * Move selection.
	 *
	 * @param direction the direction
	 * @param relationType the relation type
	 */
	public void moveSelection(GotoDirection direction, RelationType relationType) {
		waveformPane.moveSelection(direction, relationType);
	}

	/**
	 * Move cursor.
	 *
	 * @param direction the direction
	 */
	public void moveCursor(GotoDirection direction) {
		waveformPane.moveCursor(direction);
	}

	/**
	 * Sets the zoom level.
	 *
	 * @param level the new zoom level
	 */
	public void setZoomLevel(Integer level) {
		if (level < 0)
			level = 0;
		if (level > zoomLevel.length - 1)
			level = zoomLevel.length - 1;
		waveformPane.setZoomLevel(level);
		updateAll();
	}

	/**
	 * Sets the zoom fit.
	 */
	public void setZoomFit() {
		waveformPane.setZoomLevel(6);
		updateAll();
	}

	/**
	 * Gets the zoom level.
	 *
	 * @return the zoom level
	 */
	public int getZoomLevel() {
		return waveformPane.getZoomLevel();
	}

	/**
	 * Gets the selection.
	 *
	 * @return the selection
	 */
	public ISelection getSelection() {
		return waveformPane.getSelection();
	}

	/**
	 * Sets the selection.
	 *
	 * @param structuredSelection the new selection
	 */
	public void setSelection(IStructuredSelection structuredSelection) {
		waveformPane.setSelection(structuredSelection, true);
	}

	/**
	 * Gets the scaled time.
	 *
	 * @param time the time
	 * @return the scaled time
	 */
	public String getScaledTime(Long time) {
		return waveformPane.getScaledTime(time);
	}

	/**
	 * Store design brower state.
	 *
	 * @param browserState the browser state
	 */
	public void storeDesignBrowerState(Object browserState) {
		this.browserState=browserState;
	}

	/**
	 * Retrieve design brower state.
	 *
	 * @return the object
	 */
	public Object retrieveDesignBrowerState() {
		return browserState;
	}

	/**
	 * Store transaction details settings
	 *
	 * @param detailsSettings the details settings
	 */
	public void storeDetailsSettings(Object detailsSettings) {
		this.detailsSettings=detailsSettings;
	}

	/**
	 * Retrieve design details settings.
	 *
	 * @return the details settings
	 */
	public Object retrieveDetailsSettings() {
		return detailsSettings;
	}

	/**
	 * Gets the all relation types.
	 *
	 * @return the all relation types
	 */
	public List<RelationType> getAllRelationTypes() {
		List<RelationType> res =new ArrayList<>();
		res.add(IWaveformViewer.NEXT_PREV_IN_STREAM);
		res.addAll(database.getAllRelationTypes());
		return res;
	}

	/**
	 * Gets the selection relation types.
	 *
	 * @return the selection relation types
	 */
	public List<RelationType> getSelectionRelationTypes() {
		List<RelationType> res =new ArrayList<>();
		res.add(IWaveformViewer.NEXT_PREV_IN_STREAM);
		ISelection selection = waveformPane.getSelection();
		if(selection instanceof IStructuredSelection && !selection.isEmpty()){
			IStructuredSelection sel=(IStructuredSelection) selection;
			if(sel.getFirstElement() instanceof ITx){
				ITx tx = (ITx) sel.getFirstElement();
				for(ITxRelation rel:tx.getIncomingRelations()){
					if(!res.contains(rel.getRelationType()))
						res.add(rel.getRelationType());
				}
				for(ITxRelation rel:tx.getOutgoingRelations()){
					if(!res.contains(rel.getRelationType()))
						res.add(rel.getRelationType());
				}
			}
		}
		return res;
	}

	/**
	 * Gets the relation type filter.
	 *
	 * @return the relation type filter
	 */
	public RelationType getRelationTypeFilter() {
		return navigationRelationType;
	}

	/**
	 * Sets the navigation relation type.
	 *
	 * @param relationName the new navigation relation type
	 */
	public void setNavigationRelationType(String relationName) {
		setNavigationRelationType(RelationType.create(relationName));
	}

	/**
	 * Sets the navigation relation type.
	 *
	 * @param relationType the new navigation relation type
	 */
	public void setNavigationRelationType(RelationType relationType) {
		if(navigationRelationType!=relationType) waveformPane.setHighliteRelation(relationType);
		navigationRelationType=relationType;
	}

}