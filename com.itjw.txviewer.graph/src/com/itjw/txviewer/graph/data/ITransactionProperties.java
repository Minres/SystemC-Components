package com.itjw.txviewer.graph.data;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.IPropertySource;
import org.eclipse.ui.views.properties.PropertyDescriptor;

import com.itjw.txviewer.database.ITrAttribute;
import com.itjw.txviewer.database.ITransaction;

public class ITransactionProperties implements IPropertySource {

	private ITransaction iTransaction;

	public final static String PROPERTY_ID = "ID";
	public final static String PROPERTY_BEGINTIME = "BEGINTIME";
	public final static String PROPERTY_ENDTIME = "ENDTIME";
	
	protected IPropertyDescriptor[] propertyDescriptors;

	public ITransactionProperties(ITransaction iTransaction) {
		this.iTransaction=iTransaction;
		}

	@Override
	public Object getEditableValue() {
		return null;
	}

	@Override
	public IPropertyDescriptor[] getPropertyDescriptors() {
        if (propertyDescriptors == null) {
        	ArrayList<IPropertyDescriptor> idDescriptor=new ArrayList<IPropertyDescriptor>();
            // Create a descriptor and set a category
            PropertyDescriptor nameDescriptor = new PropertyDescriptor(PROPERTY_ID, "Id");
            nameDescriptor.setCategory("Attributes");
            idDescriptor.add(nameDescriptor);
            PropertyDescriptor begTimeDescriptor = new PropertyDescriptor(PROPERTY_BEGINTIME, "Begin time");
            begTimeDescriptor.setCategory("Attributes");
            idDescriptor.add(begTimeDescriptor);
            PropertyDescriptor endTimeDescriptor = new PropertyDescriptor(PROPERTY_ENDTIME, "End time");
            endTimeDescriptor.setCategory("Attributes");
            idDescriptor.add(endTimeDescriptor);
//            for(ITrAttribute attr:iTransaction.getBeginAttrs()){
//                PropertyDescriptor descr = new PropertyDescriptor("BEGIN_"+attr.getName(), attr.getName());
//                descr.setCategory("Begin attributes");
//                idDescriptor.add(descr);
//            }
//            for(ITrAttribute attr:iTransaction.getEndAttrs()){
//                PropertyDescriptor descr = new PropertyDescriptor("END_"+attr.getName(), attr.getName());
//                descr.setCategory("End attributes");
//                idDescriptor.add(descr);
//            }
//            for(ITrAttribute attr:iTransaction.getAttributes()){
//                PropertyDescriptor descr = new PropertyDescriptor("REC_"+attr.getName(), attr.getName());
//                descr.setCategory("Recorded attributes");
//                idDescriptor.add(descr);
//            }
            propertyDescriptors = idDescriptor.toArray(new IPropertyDescriptor[idDescriptor.size()]);
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
		} else if(id instanceof String){
			String strId=(String)id;
			List<ITrAttribute> set;
			if(strId.startsWith("BEGIN_")){
				set=iTransaction.getBeginAttrs();
			} else if(strId.startsWith("END_")){
				set=iTransaction.getEndAttrs();
			} else {
				set=iTransaction.getAttributes();
			}
			for(ITrAttribute attr:set){
				if(strId.endsWith("_"+attr.getName())){
					return attr.getValue();
				}
			}
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
