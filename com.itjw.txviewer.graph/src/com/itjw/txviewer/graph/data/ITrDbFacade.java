package com.itjw.txviewer.graph.data;

import java.io.InputStream;
import java.util.Arrays;
import java.util.List;

import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.PropertyDescriptor;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.ServiceReference;

import com.itjw.txviewer.database.EventTime;
import com.itjw.txviewer.database.ITrDb;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.InputFormatException;

public class ITrDbFacade extends ITrHierNodeFacade implements ITrDb {

	protected final static String PROPERTY_DURATION = "DURATIONS";
	protected final static String PROPERTY_STREAMS = "NR_OF_STREAMS";

	public static final String DATABASE = "DATABASE";
		
	public ITrDbFacade() {
		super(null);
	}

	private ITrDb getDb(){
		return (ITrDb)iTrHierNode;
	}
	
	@Override
	public EventTime getMaxTime() {
		return getDb().getMaxTime();
	}

	@Override
	public List<ITrStream> getAllStreams() {
		return getDb().getAllStreams();
	}

	@Override
	public void load(InputStream input) throws InputFormatException {
		BundleContext context = FrameworkUtil.getBundle(this.getClass()).getBundleContext();
		ServiceReference<?> serviceReference = context.getServiceReference(ITrDb.class.getName());
		ITrDb iTrDb = (ITrDb) context.getService(serviceReference);
		iTrDb.load(input);
		iTrHierNode=iTrDb;
		firePropertyChange(DATABASE, null, getDb()); 
	}

	@Override
	public void clear() {
		firePropertyChange(DATABASE, getDb(), null);
		getDb().clear();
	}

	@Override
	public IPropertyDescriptor[] getPropertyDescriptors() {
        if (propertyDescriptors == null) {
    		super.getPropertyDescriptors();
            // Create a descriptor and set a category
            PropertyDescriptor nrDescriptor = new PropertyDescriptor(PROPERTY_DURATION, "Duration");
            nrDescriptor.setCategory("Hier Node");
            PropertyDescriptor streamsDescriptor = new PropertyDescriptor(PROPERTY_STREAMS, "Stream count");
            nrDescriptor.setCategory("Hier Node");
            IPropertyDescriptor[] result = Arrays.copyOf(propertyDescriptors, propertyDescriptors.length + 2);
            System.arraycopy(new IPropertyDescriptor[] {nrDescriptor, streamsDescriptor}, 0, result, propertyDescriptors.length, 2);
            propertyDescriptors = result;
        }
        return propertyDescriptors;
	}

	@Override
	public Object getPropertyValue(Object id) {
		if (id.equals(PROPERTY_DURATION)) {
			return getMaxTime().getValueNS();
		} else if (id.equals(PROPERTY_STREAMS)) {
				return getAllStreams().size();
		} else {
			return super.getPropertyValue(id);
		}
	}

}
