
import { useState } from "react";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Badge } from "@/components/ui/badge";
import { Search, Plus, FileText } from "lucide-react";

// Mock case data
const mockCases = [
  {
    id: "1",
    name: "Smith v. Jones",
    number: "CV-2024-0589",
    client: "Robert Smith",
    status: "Active",
    type: "Litigation",
    unread: 3,
    documents: 24,
    updated: "Today"
  },
  {
    id: "2",
    name: "Brighton Estates",
    number: "RE-2023-1245",
    client: "Brighton Properties LLC",
    status: "Active",
    type: "Real Estate",
    unread: 0,
    documents: 17,
    updated: "Yesterday"
  },
  {
    id: "3",
    name: "ClientCorp Merger",
    number: "M&A-2024-0038",
    client: "ClientCorp Inc.",
    status: "Pending",
    type: "Corporate",
    unread: 5,
    documents: 31,
    updated: "Apr 15"
  },
  {
    id: "4",
    name: "Rodriguez Estate",
    number: "PR-2023-0752",
    client: "Maria Rodriguez",
    status: "Active",
    type: "Probate",
    unread: 0,
    documents: 9,
    updated: "Apr 14"
  },
  {
    id: "5",
    name: "TechStart IP Dispute",
    number: "IP-2024-0126",
    client: "TechStart Inc.",
    status: "Inactive",
    type: "Intellectual Property",
    unread: 0,
    documents: 42,
    updated: "Mar 28"
  }
];

interface CaseCardProps {
  caseData: typeof mockCases[0];
  onClick: (id: string) => void;
}

const CaseCard = ({ caseData, onClick }: CaseCardProps) => {
  return (
    <div
      className="border rounded-md p-4 hover:shadow-md transition-shadow cursor-pointer bg-white"
      onClick={() => onClick(caseData.id)}
    >
      <div className="flex justify-between items-start mb-2">
        <h3 className="font-medium text-lg">{caseData.name}</h3>
        <Badge
          variant={caseData.status === "Active" ? "default" : "outline"}
          className={caseData.status === "Active" ? "bg-green-600" : caseData.status === "Pending" ? "bg-amber-500" : ""}
        >
          {caseData.status}
        </Badge>
      </div>
      
      <div className="text-sm text-muted-foreground mb-3">
        {caseData.number} • {caseData.type}
      </div>
      
      <div className="text-sm mb-4">
        <span className="font-medium">Client:</span> {caseData.client}
      </div>
      
      <div className="flex justify-between items-center mt-2 text-sm">
        <div className="flex items-center gap-3">
          <div className="flex items-center gap-1">
            <FileText className="h-4 w-4 text-muted-foreground" />
            <span>{caseData.documents}</span>
          </div>
          
          {caseData.unread > 0 && (
            <Badge variant="secondary" className="bg-legal-navy text-white">
              {caseData.unread} new
            </Badge>
          )}
        </div>
        
        <div className="text-muted-foreground">
          Updated: {caseData.updated}
        </div>
      </div>
    </div>
  );
};

export function CaseList() {
  const [searchQuery, setSearchQuery] = useState("");
  
  const handleCaseClick = (id: string) => {
    console.log("Opening case:", id);
    // Logic to navigate to case details
  };

  return (
    <div className="h-full flex flex-col">
      <div className="p-4 border-b flex flex-col gap-4">
        <div className="flex gap-2 items-center justify-between">
          <h2 className="text-xl font-semibold">Cases</h2>
          <Button>
            <Plus className="h-4 w-4 mr-2" />
            New Case
          </Button>
        </div>
        
        <div className="relative">
          <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-muted-foreground" />
          <Input
            placeholder="Search cases..."
            className="pl-10"
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        </div>
      </div>
      
      <div className="flex-1 overflow-auto p-4">
        <div className="grid gap-4 md:grid-cols-1 lg:grid-cols-2">
          {mockCases.map((caseItem) => (
            <CaseCard
              key={caseItem.id}
              caseData={caseItem}
              onClick={handleCaseClick}
            />
          ))}
        </div>
      </div>
    </div>
  );
}
