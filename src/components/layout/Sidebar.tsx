
import { useState } from "react";
import { Link } from "react-router-dom";
import { cn } from "@/lib/utils";
import { Button } from "@/components/ui/button";
import { Inbox, FileText, Search, Users, Calendar, Settings, Menu, X } from "lucide-react";

interface SidebarLinkProps {
  to: string;
  icon: React.ComponentType<React.SVGProps<SVGSVGElement>>;
  label: string;
  isActive?: boolean;
}

const SidebarLink = ({ to, icon: Icon, label, isActive }: SidebarLinkProps) => (
  <Link to={to}>
    <Button
      variant="ghost"
      className={cn(
        "w-full justify-start gap-2 mb-1",
        isActive ? "bg-legal-light text-legal-navy font-medium" : "hover:bg-legal-light/50"
      )}
    >
      <Icon className="h-5 w-5" />
      <span>{label}</span>
    </Button>
  </Link>
);

export function Sidebar() {
  const [isCollapsed, setIsCollapsed] = useState(false);

  return (
    <div
      className={cn(
        "h-screen bg-white border-r border-border transition-all duration-300 flex flex-col",
        isCollapsed ? "w-16" : "w-64"
      )}
    >
      <div className="p-4 flex items-center justify-between border-b">
        {!isCollapsed && (
          <div className="font-semibold text-lg text-legal-navy">CaseMail Connect</div>
        )}
        <Button
          variant="ghost"
          size="icon"
          className="ml-auto"
          onClick={() => setIsCollapsed(!isCollapsed)}
        >
          {isCollapsed ? <Menu size={20} /> : <X size={20} />}
        </Button>
      </div>

      <div className="flex-1 overflow-auto p-3">
        <div className="space-y-1">
          {!isCollapsed ? (
            <>
              <SidebarLink to="/" icon={Inbox} label="Inbox" isActive />
              <SidebarLink to="/cases" icon={FileText} label="Cases" />
              <SidebarLink to="/search" icon={Search} label="Search" />
              <SidebarLink to="/clients" icon={Users} label="Clients" />
              <SidebarLink to="/calendar" icon={Calendar} label="Calendar" />
            </>
          ) : (
            <>
              <Button variant="ghost" size="icon" className="w-full mb-1">
                <Inbox size={20} />
              </Button>
              <Button variant="ghost" size="icon" className="w-full mb-1">
                <FileText size={20} />
              </Button>
              <Button variant="ghost" size="icon" className="w-full mb-1">
                <Search size={20} />
              </Button>
              <Button variant="ghost" size="icon" className="w-full mb-1">
                <Users size={20} />
              </Button>
              <Button variant="ghost" size="icon" className="w-full mb-1">
                <Calendar size={20} />
              </Button>
            </>
          )}
        </div>
      </div>

      <div className="border-t p-3">
        {!isCollapsed ? (
          <SidebarLink to="/settings" icon={Settings} label="Settings" />
        ) : (
          <Button variant="ghost" size="icon" className="w-full">
            <Settings size={20} />
          </Button>
        )}
      </div>
    </div>
  );
}
