#
# Created on Sun Mar 06 2022
#
# profile_cloudlab.py
# Copyright (C) 2022 
# 
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.
#

"""This profile demonstrates how to add some extra *local* disk space on your
node. In general nodes have much more disk space then what you see with `df`
when you log in. That extra space is in unallocated partitions or additional
disk drives. An *ephemeral blockstore* is how you ask for some of that space to
be allocated and mounted as a **temporary** filesystem (temporary means it will
be lost when you terminate your experiment).

Instructions:
Log into your node, your **temporary** file system in mounted at `/mydata`.
"""

# Import the Portal object.
import geni.portal as portal
# Import the ProtoGENI library.
import geni.rspec.pg as rspec
# Import the emulab extensions library.
import geni.rspec.emulab

# Create a Request object to start building the RSpec.
request = portal.context.makeRequestRSpec()

pc = portal.Context()
# Variable number of nodes.
pc.defineParameter("nodeCount", "Number of Nodes", portal.ParameterType.INTEGER, 2,
                   longDescription="If you specify more then one node, " +
                   "we will create a lan for you.")
# Optional physical type for all nodes.
pc.defineParameter("phystype",  "Optional physical node type",
                   portal.ParameterType.STRING, "c6420",
                   longDescription="Specify a physical node type (pc3000,d710,etc) " +
                   "instead of letting the resource mapper choose for you.")
pc.defineParameter("DATASET", "URN of your image-backed dataset", 
                   portal.ParameterType.STRING,
                   "urn:publicid:IDN+clemson.cloudlab.us:dsdm-pg0+imdataset+ImgData-Clemson")
pc.defineParameter("MPOINT", "Mountpoint for file system",
                   portal.ParameterType.STRING, "/mydata")
# For very large lans you might to tell the resource mapper to override the bandwidth constraints
# and treat it a "best-effort"
pc.defineParameter("bestEffort",  "Best Effort", portal.ParameterType.BOOLEAN, True,
                    advanced=True,
                    longDescription="For very large lans, you might get an error saying 'not enough bandwidth.' " +
                    "This options tells the resource mapper to ignore bandwidth and assume you know what you " +
                    "are doing, just give me the lan I ask for (if enough nodes are available).")                   

params = pc.bindParameters()

#node = request.RawPC("mynode")
from datetime import datetime, timedelta
import calendar
now = datetime.now()
timestamp = calendar.timegm(now.timetuple())
local_dt = datetime.fromtimestamp(timestamp - (3600 * 8)) #DFZ: 8 hours behind UTC
node_localtime = local_dt.replace(microsecond=now.microsecond)

# Create link/lan.
if params.nodeCount > 1:
    if params.nodeCount == 2:
        lan = request.Link()
    else:
        lan = request.LAN()
    if params.bestEffort:
        lan.best_effort = True
    # elif params.linkSpeed > 0:
    #     lan.bandwidth = params.linkSpeed
    # if params.sameSwitch:
    #     lan.setNoInterSwitchLinks()

# Process nodes, adding to link or lan.
for i in range(params.nodeCount):
    # Create a node and add it to the request
    # name = params.phystype + "-" + str(i) + node_localtime.strftime("-%m%d-%H%M")
    node = request.RawPC(params.phystype + "-node" + str(i))

    # Add to lan
    if params.nodeCount > 1:
        iface = node.addInterface("eth1")
        lan.addInterface(iface)

    # Optional hardware type.
    if params.phystype != "":
        node.hardware_type = params.phystype

    # Load OS and image dataset
    node.disk_image = 'urn:publicid:IDN+emulab.net+image+emulab-ops//UBUNTU20-64-STD'
    bs = node.Blockstore("bs"+str(i), params.MPOINT)
    bs.dataset = params.DATASET

# Print the RSpec to the enclosing page.
portal.context.printRequestRSpec()
